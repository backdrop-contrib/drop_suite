<?php
/**
 * @file
 * Front-end page callbacks for Drop Pay — return + cancel from gateways.
 */

/**
 * Page callback for /drop/pay/return/{pid}.
 *
 * Called by the user's browser when they return from the gateway. We ask the
 * gateway adapter to confirm the payment server-side (the gateway's confirm
 * callback is responsible for calling the gateway's API and returning TRUE
 * if the charge actually succeeded).
 *
 * If the payment has a return_path stored in metadata, we redirect there
 * after marking the payment. Otherwise we render a simple status page.
 */
function drop_pay_return_page(DropPayment $payment) {
  $gateway = drop_pay_get_gateway($payment->gateway);

  if (!$gateway) {
    backdrop_set_message(t('Unknown payment gateway: @g', array('@g' => $payment->gateway)), 'error');
    return _drop_pay_status_render($payment);
  }

  // Skip the confirm round-trip if the payment is already in a terminal state.
  if (!in_array($payment->status, array('completed', 'refunded', 'failed', 'cancelled'), TRUE)) {
    $callback = $gateway['confirm'];
    if (function_exists($callback)) {
      try {
        $confirmed = (bool) $callback($payment);
      }
      catch (Exception $e) {
        watchdog('drop_pay', 'Gateway confirm threw: @msg', array('@msg' => $e->getMessage()), WATCHDOG_ERROR);
        drop_pay_set_status($payment, 'failed', array('error' => $e->getMessage()));
        $confirmed = FALSE;
      }

      if ($confirmed && $payment->status !== 'completed') {
        drop_pay_set_status($payment, 'completed');
      }
      elseif (!$confirmed && $payment->status === 'pending') {
        // Leave pending — webhook may still finalize it. Don't flip to failed
        // here in case the user just got back faster than the gateway notification.
      }
    }
  }

  // If the consumer asked for a redirect on success, honor it.
  if ($payment->status === 'completed') {
    $metadata = is_array($payment->metadata) ? $payment->metadata : array();
    if (!empty($metadata['return_path'])) {
      backdrop_set_message(t('Payment received. Thank you!'));
      backdrop_goto($metadata['return_path']);
    }
  }

  return _drop_pay_status_render($payment);
}

/**
 * Page callback for /drop/pay/cancel/{pid}.
 *
 * Called when the user backs out of the gateway checkout. Marks the payment
 * cancelled (only if still pending) and redirects to the consumer's return
 * path if one was supplied.
 */
function drop_pay_cancel_page(DropPayment $payment) {
  if ($payment->status === 'pending') {
    drop_pay_set_status($payment, 'cancelled');
  }

  $metadata = is_array($payment->metadata) ? $payment->metadata : array();
  $back = !empty($metadata['cancel_path'])
    ? $metadata['cancel_path']
    : (!empty($metadata['return_path']) ? $metadata['return_path'] : '<front>');

  backdrop_set_message(t('Payment was cancelled.'), 'warning');
  backdrop_goto($back);
}

/**
 * Renders a simple status page for the payment.
 */
function _drop_pay_status_render(DropPayment $payment) {
  $messages = array(
    'pending'   => t('Your payment is still being processed. We will update this page once the gateway confirms.'),
    'completed' => t('Payment received. Thank you!'),
    'failed'    => t('The payment did not succeed. Please try again or contact us.'),
    'refunded'  => t('This payment has been refunded.'),
    'cancelled' => t('This payment was cancelled.'),
  );
  $msg = isset($messages[$payment->status]) ? $messages[$payment->status] : t('Status: @s', array('@s' => $payment->status));

  $build = array();
  $build['status'] = array(
    '#type'   => 'markup',
    '#markup' => '<p class="drop_pay-return-status drop_pay-return-status--' . check_plain($payment->status) . '">' . $msg . '</p>',
  );
  $build['summary'] = array(
    '#type'   => 'markup',
    '#markup' => '<p>' . t('Reference #@pid — @desc — @amt @cur', array(
      '@pid'  => $payment->pid,
      '@desc' => $payment->description,
      '@amt'  => number_format((float) $payment->amount, 2),
      '@cur'  => $payment->currency,
    )) . '</p>',
  );
  return $build;
}
