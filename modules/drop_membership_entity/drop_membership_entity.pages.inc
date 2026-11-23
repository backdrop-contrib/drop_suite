<?php
/**
 * @file
 * Drop Suite pages for Membership Entity.
 */

/**
 * Page callback: drop/memberships.
 *
 * Shows Membership Entity's own memberships View in the Drop Suite layout.
 */
function drop_membership_entity_list_page() {
  backdrop_set_title(t('Memberships'));
  $output = views_embed_view('membership_entity_memberships', 'default');
  return array(
    '#markup' => $output ? $output : '<p>' . t('The memberships list is not available. Check that the Membership Entity view %view exists.', array('%view' => 'membership_entity_memberships')) . '</p>',
  );
}

/**
 * Page callback: drop/memberships/add[/TYPE].
 *
 * With one membership type, shows its add form straight away; with several,
 * lists them to choose from.
 *
 * @param string|null $type
 *   The membership type machine name, from the URL.
 */
function drop_membership_entity_add_page($type = NULL) {
  module_load_include('inc', 'membership_entity', 'membership_entity.pages');

  $types = array();
  if (module_exists('membership_entity_type')) {
    foreach (membership_entity_type_get_types() as $info) {
      $types[$info->type] = $info->label;
    }
  }
  else {
    $types['membership'] = t('Membership');
  }

  if ($type !== NULL) {
    $type = str_replace('-', '_', $type);
    if (!isset($types[$type])) {
      return MENU_NOT_FOUND;
    }
  }
  elseif (count($types) === 1) {
    $type = key($types);
  }

  if ($type !== NULL) {
    backdrop_set_title(t('Add a membership: @type', array('@type' => $types[$type])));
    return backdrop_get_form('membership_entity_edit_form', NULL, $type);
  }

  $items = array();
  foreach ($types as $name => $label) {
    $items[] = l($label, 'drop/memberships/add/' . str_replace('_', '-', $name));
  }
  return array(
    '#theme' => 'item_list',
    '#items' => $items,
    '#title' => t('Choose a membership type'),
  );
}

/**
 * Page callback: membership/N/unfinished-payment/continue.
 *
 * Checks the payment with the gateway first. If it was paid after all, the
 * membership is updated and the person goes to their membership. Otherwise
 * they are sent back to the checkout.
 *
 * @param object $membership
 *   The membership, from the URL.
 */
function drop_membership_entity_continue_payment_page($membership) {
  $payment = _drop_membership_entity_pending_payment($membership);
  if (!$payment) {
    backdrop_set_message(t('There is no unfinished payment for this membership.'));
    backdrop_goto('membership/' . $membership->mid);
  }

  if (drop_pay_refresh($payment) === 'completed') {
    backdrop_set_message(t('Your payment has been received. Thank you.'));
    backdrop_goto('membership/' . $membership->mid);
  }

  $metadata = is_array($payment->metadata) ? $payment->metadata : array();
  if (!empty($metadata['redirect_url'])) {
    backdrop_goto($metadata['redirect_url']);
  }

  backdrop_set_message(t('This payment cannot be continued. Cancel it and start again.'), 'warning');
  backdrop_goto('membership/' . $membership->mid . '/unfinished-payment/cancel');
}

/**
 * Form: membership/N/unfinished-payment/cancel.
 *
 * @param object $membership
 *   The membership, from the URL.
 */
function drop_membership_entity_cancel_payment_form($form, &$form_state, $membership) {
  $payment = _drop_membership_entity_pending_payment($membership);
  if (!$payment) {
    backdrop_set_message(t('There is no unfinished payment for this membership.'));
    backdrop_goto('membership/' . $membership->mid);
  }

  backdrop_set_title(t('Cancel unfinished payment'));
  $form_state['membership_mid'] = $membership->mid;
  $form_state['payment_pid'] = $payment->pid;

  $form['help'] = array(
    '#type'   => 'markup',
    '#markup' => '<p>' . t('This cancels the payment of @amount started on @date, so you can start again. Stripe is asked first: if the payment has already gone through, it is completed instead and nothing is cancelled.', array(
      '@amount' => _drop_membership_entity_format_amount($payment->amount),
      '@date'   => format_date($payment->created, 'short'),
    )) . '</p>',
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Cancel payment'),
  );
  $form['actions']['keep'] = array(
    '#type'  => 'link',
    '#title' => t('Keep it'),
    '#href'  => 'membership/' . $membership->mid,
  );
  return $form;
}

/**
 * Submit handler: cancels the unfinished payment, after checking with Stripe.
 */
function drop_membership_entity_cancel_payment_form_submit($form, &$form_state) {
  $mid = $form_state['membership_mid'];
  $payment = drop_payment_load($form_state['payment_pid']);
  $membership = membership_entity_load($mid, TRUE);
  // A cancelled join removes its membership, so the page may no longer exist.
  $destination = $membership ? 'membership/' . $mid : '<front>';

  switch (drop_pay_abandon($payment)) {
    case 'cancelled':
      backdrop_set_message(t('The unfinished payment is cancelled. You can renew or join again.'));
      break;

    case 'completed':
      backdrop_set_message(t('The payment had already gone through, so nothing was cancelled. Thank you.'));
      break;

    case 'unchanged':
      backdrop_set_message(t('That payment is no longer pending.'));
      break;

    default:
      backdrop_set_message(t('The payment provider could not be reached, so nothing was cancelled. Please try again in a few minutes.'), 'error');
      $destination = 'membership/' . $mid . '/unfinished-payment/cancel';
      break;
  }
  $form_state['redirect'] = $destination;
}
