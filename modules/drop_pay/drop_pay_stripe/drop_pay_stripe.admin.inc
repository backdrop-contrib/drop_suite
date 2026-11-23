<?php
/**
 * @file
 * Settings form for the Drop Pay Stripe gateway.
 */

/**
 * Form callback: drop/pay/settings/gateway/stripe.
 *
 * Stripe credentials live in the contrib stripe module. This page is mostly
 * informational + offers a one-click jump to that settings form, plus the
 * webhook URL that needs to be registered in the Stripe dashboard.
 */
function drop_pay_stripe_settings_form($form, &$form_state) {
  $form['intro'] = array(
    '#type'   => 'markup',
    '#markup' => '<p>' . t('Drop Pay Stripe uses the contrib %stripe module for credentials and SDK loading. This page is a quick reference; set your API keys on the @link page.', array(
      '%stripe' => 'Stripe',
      '@link'   => l(t('Stripe settings'), 'admin/config/services/stripe'),
    )) . '</p>',
  );

  $mode = config_get('stripe.settings', 'mode');
  $secret_set = (bool) stripe_get_secret_key();
  $public_set = (bool) stripe_get_public_key();

  $rows = array();
  $rows[] = array(t('Active mode'), $mode ? check_plain($mode) : '<em>' . t('not set') . '</em>');
  $rows[] = array(t('Secret key on file'), $secret_set ? t('Yes') : '<strong>' . t('No — set in Stripe settings') . '</strong>');
  $rows[] = array(t('Public key on file'), $public_set ? t('Yes') : '<strong>' . t('No — set in Stripe settings') . '</strong>');
  $rows[] = array(t('Webhook URL'), '<code>' . url('stripe/webhook', array('absolute' => TRUE)) . '</code>');
  $form['status'] = array(
    '#type'  => 'item',
    '#title' => t('Stripe status'),
    '#markup' => theme('table', array('header' => array(), 'rows' => $rows)),
  );

  $form['help'] = array(
    '#type'   => 'markup',
    '#markup' => '<p>' . t('In your Stripe dashboard, add the webhook URL above and subscribe to %events. These events keep payment status in sync even when the payer closes their browser before being redirected back.', array(
      '%events' => 'checkout.session.completed, charge.refunded',
    )) . '</p>',
  );

  // No saveable settings here yet — present a single "Done" button that just
  // bounces back to the Drop Pay settings page.
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['back'] = array(
    '#type'   => 'link',
    '#title'  => t('Back to Drop Pay settings'),
    '#href'   => 'drop/pay/settings',
    '#options' => array('attributes' => array('class' => array('button'))),
  );

  return $form;
}
