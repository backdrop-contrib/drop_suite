<?php
/**
 * @file
 * Entity classes for Drop Pay.
 */

/**
 * Entity class for drop_payment.
 */
class DropPayment extends Entity {

  public function id() {
    return isset($this->pid) ? $this->pid : NULL;
  }

  public function entityType() {
    return 'drop_payment';
  }

  public function bundle() {
    return 'drop_payment';
  }

  public function label() {
    $amount = number_format((float) (isset($this->amount) ? $this->amount : 0), 2);
    $currency = !empty($this->currency) ? $this->currency : 'USD';
    $purpose = !empty($this->description) ? $this->description : (!empty($this->purpose) ? $this->purpose : t('Payment'));
    return t('!purpose — !amount !currency', array(
      '!purpose'  => $purpose,
      '!amount'   => $amount,
      '!currency' => $currency,
    ));
  }

  public function uri() {
    return array('path' => 'drop/pay/settings/payment/' . $this->id(), 'options' => array());
  }

}

/**
 * Controller for drop_payment.
 */
class DropPaymentController extends EntityPlusController {
}
