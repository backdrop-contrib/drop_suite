<?php
/**
 * @file
 * Hooks provided by Drop Pay for gateway adapters and consumers.
 */

/**
 * Defines a payment gateway.
 *
 * Implementing modules return an array keyed by gateway machine name. Each
 * gateway provides callbacks that drop_pay invokes during the payment
 * lifecycle.
 *
 * @return array
 *   array(
 *     'mygateway' => array(
 *       'label'         => t('My Gateway'),
 *       'description'   => t('Optional one-line description shown on settings page.'),
 *       // REQUIRED. Called by drop_pay_charge() to start a payment.
 *       // Signature: function (DropPayment $payment, array $request)
 *       // Returns: array('redirect_url' => '...') OR a structured response.
 *       // Send the payer back with drop_pay_payment_url('return', $payment)
 *       // and drop_pay_payment_url('cancel', $payment): those URLs carry
 *       // the token that lets the payer (and only the payer) open them.
 *       'initiate'      => 'mygateway_initiate',
 *       // REQUIRED. Asks the gateway's API whether the payment was paid.
 *       // Used by the return page, drop_pay_refresh() and
 *       // drop_pay_abandon(). Never trust data the payer's browser or an
 *       // unverified webhook sent; ask the gateway.
 *       // Signature: function (DropPayment $payment)
 *       // Returns: TRUE if paid, FALSE if not. Throw on errors (the
 *       // payment then stays pending).
 *       'confirm'       => 'mygateway_confirm',
 *       // OPTIONAL. Programmatic refund from the admin UI.
 *       // Signature: function (DropPayment $payment, $amount = NULL)
 *       // Returns: TRUE on success.
 *       'refund'        => 'mygateway_refund',
 *       // OPTIONAL, but without it drop_pay_abandon() (and so the cancel
 *       // page) can't cancel a payment: the checkout would stay payable.
 *       // Signature: function (DropPayment $payment). Throw on failure.
 *       'expire'        => 'mygateway_expire',
 *       // OPTIONAL. Settings form callback for gateway configuration.
 *       // Returned as a regular form callback for backdrop_get_form().
 *       'settings_form' => 'mygateway_settings_form',
 *     ),
 *   )
 */
function hook_drop_pay_gateways() {
  return array(
    'mygateway' => array(
      'label'         => t('My Gateway'),
      'initiate'      => 'mygateway_initiate',
      'confirm'       => 'mygateway_confirm',
      'refund'        => 'mygateway_refund',
      'settings_form' => 'mygateway_settings_form',
    ),
  );
}

/**
 * React to a payment changing state.
 *
 * Fired from drop_pay_set_status() after the payment row is saved, once per
 * real status change. Only the changes in drop_pay_status_transitions() are
 * made: pending to completed, failed or cancelled, and completed to refunded.
 * Consumer modules use this to react to completion (create related records,
 * send receipts, advance state machines, etc.).
 *
 * Offline payments recorded with drop_pay_record_payment() fire this hook
 * too (pending -> completed), with $payment->gateway set to DROP_PAY_MANUAL
 * and the method and reference in $payment->metadata.
 *
 * @param DropPayment $payment
 *   The payment entity, as just saved.
 * @param string $previous_status
 *   The status the payment had before this transition.
 */
function hook_drop_pay_payment_status(DropPayment $payment, $previous_status) {
  if ($payment->status === 'completed' && $previous_status !== 'completed') {
    // A payment completed — do something with it.
    if ($payment->purpose === 'dues' && $payment->purpose_entity_type === 'crm_membership_term') {
      // Create the membership-side payment record, advance the term, etc.
    }
  }
}
