<?php
/**
 * @file
 * Admin pages for Drop Pay.
 */

// ---------------------------------------------------------------------------
// Settings form
// ---------------------------------------------------------------------------

/**
 * Form callback: drop/pay/settings.
 */
function drop_pay_settings_form($form, &$form_state) {
  $form['#config'] = 'drop_pay.settings';

  $gateways = drop_pay_get_gateways();
  $options = array('' => t('- None -'));
  foreach ($gateways as $name => $gw) {
    $options[$name] = $gw['label'];
  }

  $form['default_gateway'] = array(
    '#type'          => 'radios',
    '#title'         => t('Default payment gateway'),
    '#options'       => $options,
    '#default_value' => config_get('drop_pay.settings', 'default_gateway'),
    '#description'   => t('The gateway used when a consumer module does not specify one. Each registered gateway also has its own settings page (Drop Pay > the gateway name).'),
  );

  if (count($gateways) === 0) {
    $form['no_gateways'] = array(
      '#type'   => 'markup',
      '#markup' => '<p class="messages warning">' . t('No payment gateways are registered. Enable a gateway submodule like Drop Pay Stripe to begin accepting payments.') . '</p>',
    );
  }
  else {
    $items = array();
    foreach ($gateways as $name => $gw) {
      $label = check_plain($gw['label']);
      if (!empty($gw['settings_form'])) {
        $items[] = l($label, 'drop/pay/settings/gateway/' . $name);
      }
      else {
        $items[] = $label;
      }
    }
    $form['registered'] = array(
      '#type'   => 'item',
      '#title'  => t('Registered gateways'),
      '#markup' => theme('item_list', array('items' => $items)),
    );
  }

  $lines = array();
  foreach (drop_pay_manual_methods() as $key => $label) {
    $lines[] = $key . '|' . $label;
  }
  $form['manual_methods'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Offline payment methods'),
    '#default_value' => implode("\n", $lines),
    '#rows'          => 5,
    '#description'   => t('Ways staff can record a payment received outside an online gateway. One per line, as <em>key|Label</em> (for example <em>check|Check</em>). The key is stored on each payment, so keep existing keys when renaming a label.'),
  );
  $form['#validate'][] = 'drop_pay_settings_form_validate';

  return system_settings_form($form);
}

/**
 * Validation handler: turns the offline methods text into a keyed array.
 */
function drop_pay_settings_form_validate($form, &$form_state) {
  $methods = array();
  $lines = preg_split('/\r\n|\r|\n/', $form_state['values']['manual_methods']);
  foreach ($lines as $line) {
    $line = trim($line);
    if ($line === '') {
      continue;
    }
    if (strpos($line, '|') !== FALSE) {
      list($key, $label) = explode('|', $line, 2);
    }
    else {
      $key = $label = $line;
    }
    $key = trim(preg_replace('/[^a-z0-9_]+/', '_', strtolower(trim($key))), '_');
    $label = trim($label);
    if ($key === '' || $label === '') {
      form_set_error('manual_methods', t('Each offline payment method needs a key and a label: %line', array('%line' => $line)));
      return;
    }
    if ($key === DROP_PAY_MANUAL) {
      form_set_error('manual_methods', t('The key %key is reserved. Choose another.', array('%key' => $key)));
      return;
    }
    $methods[$key] = $label;
  }
  if (empty($methods)) {
    form_set_error('manual_methods', t('Enter at least one offline payment method.'));
    return;
  }
  form_set_value($form['manual_methods'], $methods, $form_state);
}

// ---------------------------------------------------------------------------
// Payment list
// ---------------------------------------------------------------------------

/**
 * Page callback: drop/pay/settings/payments.
 *
 * Lightweight admin list — Views can override or supplement this once a
 * Views display is built. Filters by status via querystring.
 */
function drop_pay_payment_list_page() {
  $build = array();

  $statuses = _drop_pay_status_options();
  $current = isset($_GET['status']) && isset($statuses[$_GET['status']]) ? $_GET['status'] : '';

  // Status filter pills.
  $links = array();
  $links[] = ($current === '')
    ? '<strong>' . t('All') . '</strong>'
    : l(t('All'), 'drop/pay/settings/payments');
  foreach ($statuses as $key => $label) {
    if ($current === $key) {
      $links[] = '<strong>' . check_plain($label) . '</strong>';
    }
    else {
      $links[] = l($label, 'drop/pay/settings/payments', array('query' => array('status' => $key)));
    }
  }
  $build['filter'] = array(
    '#type'   => 'markup',
    '#markup' => '<div class="drop_pay-filter">' . implode(' | ', $links) . '</div>',
  );

  $header = array(
    array('data' => t('ID'),       'field' => 'pid', 'sort' => 'desc'),
    array('data' => t('Created'),  'field' => 'created'),
    array('data' => t('Status'),   'field' => 'status'),
    array('data' => t('Paid via'), 'field' => 'gateway'),
    array('data' => t('Purpose'),  'field' => 'purpose'),
    t('Description'),
    array('data' => t('Amount'),   'field' => 'amount'),
    t('Operations'),
  );

  $query = db_select('drop_payment', 'p')->extend('PagerDefault')->extend('TableSort');
  $query->fields('p', array('pid', 'amount', 'currency', 'status', 'gateway', 'purpose', 'description', 'created', 'metadata'));
  if ($current !== '') {
    $query->condition('p.status', $current);
  }
  $query->orderByHeader($header)->limit(50);

  $rows = array();
  $status_labels = _drop_pay_status_options();
  foreach ($query->execute() as $row) {
    $row->metadata = $row->metadata ? unserialize($row->metadata, array('allowed_classes' => FALSE)) : array();
    if (!is_array($row->metadata)) {
      $row->metadata = array();
    }
    $rows[] = array(
      l('#' . $row->pid, 'drop/pay/settings/payment/' . $row->pid),
      format_date($row->created, 'short'),
      isset($status_labels[$row->status]) ? $status_labels[$row->status] : check_plain($row->status),
      check_plain(drop_pay_payment_method_label($row)),
      check_plain($row->purpose),
      check_plain($row->description),
      number_format((float) $row->amount, 2) . ' ' . check_plain($row->currency),
      l(t('view'), 'drop/pay/settings/payment/' . $row->pid),
    );
  }

  $build['table'] = array(
    '#theme'  => 'table',
    '#header' => $header,
    '#rows'   => $rows,
    '#empty'  => t('No payments found.'),
  );
  $build['pager'] = array('#theme' => 'pager');

  return $build;
}

// ---------------------------------------------------------------------------
// Payment view
// ---------------------------------------------------------------------------

/**
 * Page callback: drop/pay/settings/payment/{pid}.
 */
function drop_pay_payment_view_page(DropPayment $payment) {
  backdrop_set_title(t('Payment #@pid', array('@pid' => $payment->pid)));

  $rows = array();
  $rows[] = array(t('Status'), check_plain($payment->status));
  $rows[] = array(t('Amount'), number_format((float) $payment->amount, 2) . ' ' . check_plain($payment->currency));
  $rows[] = array(t('Paid via'), check_plain(drop_pay_payment_method_label($payment)));
  if ($payment->gateway === DROP_PAY_MANUAL && !empty($payment->metadata['reference'])) {
    $rows[] = array(t('Reference'), check_plain($payment->metadata['reference']));
  }
  if (!empty($payment->gateway_transaction_id)) {
    $rows[] = array(t('Gateway transaction ID'), check_plain($payment->gateway_transaction_id));
  }
  $rows[] = array(t('Description'), check_plain($payment->description));
  if (!empty($payment->purpose)) {
    $rows[] = array(t('Purpose'), check_plain($payment->purpose));
  }
  if (!empty($payment->purpose_entity_type) && !empty($payment->purpose_entity_id)) {
    $rows[] = array(t('Funded'), check_plain($payment->purpose_entity_type) . ' #' . (int) $payment->purpose_entity_id);
  }
  if (!empty($payment->cid) && function_exists('crm_contact_load')) {
    $contact = crm_contact_load($payment->cid);
    if ($contact) {
      $label = method_exists($contact, 'label') ? $contact->label() : ('Contact #' . $payment->cid);
      $rows[] = array(t('Contact'), l($label, 'drop/crm/contact/' . $payment->cid));
    }
  }
  $rows[] = array(t('Initiated by'), $payment->uid ? theme('username', array('account' => user_load($payment->uid))) : t('Anonymous'));
  $rows[] = array(t('Created'), format_date($payment->created, 'medium'));
  $rows[] = array(t('Updated'), format_date($payment->changed, 'medium'));
  if (!empty($payment->completed)) {
    $rows[] = array(t('Completed'), format_date($payment->completed, 'medium'));
  }
  if ($payment->status === 'refunded' && !empty($payment->metadata['refunded_at'])) {
    $refunded = format_date($payment->metadata['refunded_at'], 'medium');
    if (!empty($payment->metadata['refunded_by']) && ($refunder = user_load($payment->metadata['refunded_by']))) {
      $refunded = t('!date by !name', array(
        '!date' => $refunded,
        '!name' => theme('username', array('account' => $refunder)),
      ));
    }
    $rows[] = array(t('Refunded'), $refunded);
  }

  $build = array();
  $build['summary'] = array(
    '#theme'  => 'table',
    '#rows'   => $rows,
    '#header' => array(),
  );

  // Raw metadata, for administrators debugging a gateway. A native
  // <details> element, so it starts closed without any JavaScript.
  if (!empty($payment->metadata) && user_access('administer drop_pay')) {
    $build['metadata'] = array(
      '#type'    => 'details',
      '#summary' => t('Technical details'),
      '#open'    => FALSE,
    );
    $build['metadata']['dump'] = array(
      '#type'   => 'markup',
      '#markup' => '<pre>' . check_plain(print_r($payment->metadata, TRUE)) . '</pre>',
    );
  }

  // Refund button — only when gateway supports it and payment is completed.
  $gateway = drop_pay_get_gateway($payment->gateway);
  $can_refund = !empty($gateway['refund']) || $payment->gateway === DROP_PAY_MANUAL;
  if ($payment->status === 'completed' && $can_refund && user_access('administer drop_pay')) {
    $build['refund'] = backdrop_get_form('drop_pay_refund_form', $payment);
  }

  return $build;
}

/**
 * Form: refund a completed payment via its gateway.
 */
function drop_pay_refund_form($form, &$form_state, DropPayment $payment) {
  $form_state['payment'] = $payment;

  if ($payment->gateway === DROP_PAY_MANUAL) {
    $form['help'] = array(
      '#type'   => 'markup',
      '#markup' => '<p>' . t('This payment was recorded offline. Marking it refunded only changes this record; return the money to the payer yourself.') . '</p>',
    );
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
      '#type'  => 'submit',
      '#value' => t('Mark as refunded'),
    );
    return $form;
  }

  $form['help'] = array(
    '#type'   => 'markup',
    '#markup' => '<p>' . t('Refund this payment via the @g gateway. Leave the amount blank to refund the full amount.', array('@g' => $payment->gateway)) . '</p>',
  );
  $form['amount'] = array(
    '#type'        => 'textfield',
    '#title'       => t('Refund amount'),
    '#size'        => 12,
    '#description' => t('Currency: @c. Maximum: @m.', array(
      '@c' => $payment->currency,
      '@m' => number_format((float) $payment->amount, 2),
    )),
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Issue refund'),
  );
  return $form;
}

/**
 * Submit handler: dispatch to the gateway's refund callback.
 */
function drop_pay_refund_form_submit($form, &$form_state) {
  global $user;
  $payment = $form_state['payment'];

  if ($payment->gateway === DROP_PAY_MANUAL) {
    drop_pay_set_status($payment, 'refunded', array(
      'refunded_by' => $user->uid,
      'refunded_at' => REQUEST_TIME,
    ));
    backdrop_set_message(t('The payment is marked as refunded.'));
    return;
  }

  $amount = trim($form_state['values']['amount']);
  $amount = ($amount === '') ? NULL : (float) $amount;

  $gateway = drop_pay_get_gateway($payment->gateway);
  if (empty($gateway['refund']) || !function_exists($gateway['refund'])) {
    backdrop_set_message(t('This gateway does not support programmatic refunds.'), 'error');
    return;
  }

  try {
    $ok = (bool) $gateway['refund']($payment, $amount);
  }
  catch (Exception $e) {
    watchdog('drop_pay', 'Refund threw: @msg', array('@msg' => $e->getMessage()), WATCHDOG_ERROR);
    backdrop_set_message(t('Refund failed: @msg', array('@msg' => $e->getMessage())), 'error');
    return;
  }

  if ($ok) {
    drop_pay_set_status($payment, 'refunded');
    backdrop_set_message(t('Refund issued.'));
  }
  else {
    backdrop_set_message(t('The gateway reported the refund did not succeed.'), 'error');
  }
}

// ---------------------------------------------------------------------------
// Record an offline payment
// ---------------------------------------------------------------------------

/**
 * Form: drop/pay/settings/payments/record.
 *
 * Records a payment received outside an online gateway (cash, check, card in
 * person). Modules that record payments for their own records (memberships,
 * events) call drop_pay_record_payment() directly instead.
 */
function drop_pay_record_payment_form($form, &$form_state) {
  backdrop_set_title(t('Record a payment'));

  $form['amount'] = array(
    '#type'      => 'textfield',
    '#title'     => t('Amount'),
    '#size'      => 12,
    '#required'  => TRUE,
    '#field_prefix' => '$',
  );
  $form['description'] = array(
    '#type'        => 'textfield',
    '#title'       => t('Description'),
    '#required'    => TRUE,
    '#maxlength'   => 255,
    '#description' => t('What the payment was for, e.g. "Annual dues, Jane Smith".'),
  );
  $form['method'] = array(
    '#type'     => 'select',
    '#title'    => t('Payment method'),
    '#options'  => drop_pay_manual_methods(),
    '#required' => TRUE,
  );
  $form['reference'] = array(
    '#type'        => 'textfield',
    '#title'       => t('Reference'),
    '#maxlength'   => 255,
    '#description' => t('Check number, receipt number or other note. Optional.'),
  );
  $form['paid_date'] = array(
    '#type'          => 'html_date',
    '#title'         => t('Date received'),
    '#default_value' => format_date(REQUEST_TIME, 'custom', 'Y-m-d', date_default_timezone()),
    '#required'      => TRUE,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Record payment'),
  );
  $form['actions']['cancel'] = array(
    '#type'  => 'link',
    '#title' => t('Cancel'),
    '#href'  => 'drop/pay/settings/payments',
  );
  return $form;
}

/**
 * Validation handler for drop_pay_record_payment_form().
 */
function drop_pay_record_payment_form_validate($form, &$form_state) {
  $amount = trim($form_state['values']['amount']);
  if ($amount !== '' && (!is_numeric($amount) || $amount <= 0)) {
    form_set_error('amount', t('Enter an amount greater than zero, without a currency sign.'));
  }
  if (!empty($form_state['values']['paid_date']) && _drop_pay_parse_date($form_state['values']['paid_date']) === NULL) {
    form_set_error('paid_date', t('Enter a valid date.'));
  }
}

/**
 * Submit handler for drop_pay_record_payment_form().
 */
function drop_pay_record_payment_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $result = drop_pay_record_payment(array(
    'amount'      => trim($values['amount']),
    'description' => $values['description'],
    'method'      => $values['method'],
    'reference'   => $values['reference'],
    'paid_date'   => _drop_pay_parse_date($values['paid_date']),
  ));

  if (!empty($result['error'])) {
    backdrop_set_message($result['error'], 'error');
    $form_state['rebuild'] = TRUE;
    return;
  }
  backdrop_set_message(t('Payment #@pid recorded.', array('@pid' => $result['pid'])));
  $form_state['redirect'] = 'drop/pay/settings/payment/' . $result['pid'];
}

/**
 * Converts a YYYY-MM-DD date into a timestamp in the site's timezone.
 *
 * Today becomes the current time; other days become noon, so the date shows
 * the same in nearby timezones.
 *
 * @param string $date
 *   A date from an html_date element.
 *
 * @return int|null
 *   A Unix timestamp, or NULL if the date can't be read.
 */
function _drop_pay_parse_date($date) {
  $timezone = new DateTimeZone(date_default_timezone());
  $parsed = DateTime::createFromFormat('!Y-m-d', (string) $date, $timezone);
  if (!$parsed || $parsed->format('Y-m-d') !== $date) {
    return NULL;
  }
  if ($date === format_date(REQUEST_TIME, 'custom', 'Y-m-d', date_default_timezone())) {
    return REQUEST_TIME;
  }
  $parsed->setTime(12, 0);
  return $parsed->getTimestamp();
}
