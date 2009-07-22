<?php
// $Id$

/**
 * @file
 * Front end functions for the DIBS module
 */

/**
 * Page with hidden form fields that redirects to the DIBS payment
 *
 * @param string $hash
 *
 * @return string
 */
function dibs_frontend_redirect_page($hash) {
  // Loading the transaction
  $data = dibs_load_transaction($hash, 'hash');
  $data['payment_price'] = (float)$data['payment_price'];

  // Making some checks to check if the transaction is valid
  if (!empty($data['api_module']) && !empty($data['api_delta'])) {
    $info = module_invoke($data['api_module'], 'dibsapi', 'info', $data['api_delta']);
    if (empty($info) || !is_array($info)) {
      drupal_set_message(t('Error redirecting to DIBS: api_module and/or api_delta is not valid!'), 'error');
      return '';
    }
  }
  elseif (!empty($data['api_module']) || !empty($data['api_delta'])) {
    drupal_set_message(t('Error redirecting to DIBS: api_module or api_delta is missing!'), 'error');
    return '';
  }
  elseif ($data['payment_status'] > 0) {
    drupal_set_message(t('Error redirecting to DIBS: the payment has already made!'), 'error');
    return '';
  }

  // Loading settings
  $settings = dibs_load_settings($data['api_module'], $data['api_delta']);

  // Getting redirect form
  $form = drupal_get_form('dibs_frontend_redirect_form', $settings, $data);

  // We are forcing the accept-charset to be ISO-8859-1, else will the order and delivery
  // info sent to DIBS be shown with wrong characters in the payment window and in their
  // administration system.
  $form = str_replace('accept-charset="UTF-8"', 'accept-charset="ISO-8859-1"', $form);

  // Adding autosubmit javascript
  drupal_add_js("
    if (Drupal.jsEnabled) {
      $(document).ready($('#dibs-frontend-redirect-form').submit());
    }
  ", 'inline', 'footer');

  return theme('dibs_redirect_page', $form, $settings, $data);
}

/**
 * Form builder; Building the payment form and submits
 * it to DIBS with javascript
 *
 * @param array $form_state
 * @param array $settings
 * @param array $data
 *
 * @return array
 */
function dibs_frontend_redirect_form($form_state = array(), $settings, $data) {
  // Creating form fields
  // Required fields
  $form['merchant'] = array(
    '#type' => 'hidden',
    '#value' => $settings['general']['merchant'],
  );
  $form['amount'] = array(
    '#type' => 'hidden',
    '#value' => $data['payment_price'] * 100,
  );
  $form['currency'] = array(
    '#type' => 'hidden',
    '#value' => $settings['general']['currency'],
  );
  $form['lang'] = array(
    '#type' => 'hidden',
    '#value' => $settings['general']['lang'],
  );
  $form['orderid'] = array(
    '#type' => 'hidden',
    '#value' => $data['payment_order_id'],
  );
  $form['uniqueoid'] = array(
    '#type' => 'hidden',
    '#value' => 1,
  );
  $form['accepturl'] = array(
    '#type' => 'hidden',
    '#value' => url('payment/dibs/accept/' . $data['api_hash'], array('absolute' => TRUE)),
  );
  $form['cancelurl'] = array(
    '#type' => 'hidden',
    '#value' => url('payment/dibs/cancel/' . $data['api_hash'], array('absolute' => TRUE)),
  );
  $form['callbackurl'] = array(
    '#type' => 'hidden',
    '#value' => url('payment/dibs/callbackok/' . $data['api_hash'], array('absolute' => TRUE)),
  );

  // Optional fields
  if (!empty($settings['general']['test_mode'])) {
    $form['test'] = array(
      '#type' => 'hidden',
      '#value' => 1,
    );
  }
  if (!empty($settings['general']['account'])) {
    $form['account'] = array(
      '#type' => 'hidden',
      '#value' => $settings['general']['account'],
    );
  }
  if (!empty($settings['advanced']['capturenow'])) {
    $form['capturenow'] = array(
      '#type' => 'hidden',
      '#value' => 1,
    );
  }
  if (!empty($settings['advanced']['calcfee'])) {
    $form['calcfee'] = array(
      '#type' => 'hidden',
      '#value' => 1,
    );
  }
  if (!empty($settings['general']['md5'])) {
    $md5_key = md5($settings['general']['md5_key2'] . md5($settings['general']['md5_key1'] . 'merchant=' . $settings['general']['merchant'] . '&orderid=' . $data['payment_order_id'] . '&currency=' . $settings['general']['currency'] . '&amount=' . $data['payment_price'] * 100));
    $form['md5key'] = array(
      '#type' => 'hidden',
      '#value' => $md5_key,
    );
  }

  // Payment Window fields
  if ($settings['general']['type'] == 'pay') {
    if (!empty($settings['paymentwindow']['color'])) {
      $form['color'] = array(
        '#type' => 'hidden',
        '#value' => $settings['paymentwindow']['color'],
      );
    }
    $form['#action'] = 'https://payment.architrade.com/payment/start.pml';
  }
  // Flex Window fields
  elseif ($settings['general']['type'] == 'flex') {
    if (!empty($settings['flexwindow']['color'])) {
      $form['color'] = array(
        '#type' => 'hidden',
        '#value' => $settings['flexwindow']['color'],
      );
    }
    if (!empty($settings['flexwindow']['decorator']) && ($settings['flexwindow']['decorator'] != 'custom')) {
      $form['decorator'] = array(
        '#type' => 'hidden',
        '#value' => $settings['flexwindow']['decorator'],
      );
    }
    if (!empty($settings['flexwindow']['voucher'])) {
      $form['voucher'] = array(
        '#type' => 'hidden',
        '#value' => $settings['flexwindow']['voucher'] ? 'yes' : '',
      );
    }
    $form['#action'] = 'https://payment.architrade.com/paymentweb/start.action';
  }

  // Order and payment info fields
  if (!empty($data['order_info_short'])) {
    $form['ordertext'] = array(
      '#type' => 'hidden',
      '#value' => $data['order_info_short'],
    );
  }
  if (!empty($data['order_info_long']) && is_array($data['order_info_long'])) {
    $order_lines = dibs_create_order_line_fields($data['order_info_long']);
    $form = array_merge($form, $order_lines);
  }
  if (!empty($data['order_price_info']) && is_array($data['order_price_info'])) {
    $price_lines = dibs_create_price_line_fields($data['order_price_info']);
    $form = array_merge($form, $price_lines);
  }

  // Delivery line fields
  $delivery_lines = dibs_create_delivery_line_fields($data);
  $form = array_merge($form, $delivery_lines);

  // Additional form fields
  $form['submit'] = array('#type' => 'submit', '#value' => t('Continue to DIBS payment'));

  return $form;
}

/**
 * The cancel page where the user has the possibility to go back to the
 * payment again
 *
 * @param string $hash
 *
 * @return string
 */
function dibs_frontend_cancel_page($hash) {
  // Loading the transaction
  $data = dibs_load_transaction($hash, 'hash');
  $data['payment_price'] = (float)$data['payment_price'];

  // Making some checks to check if the transaction is valid
  if (!empty($data['api_module']) && !empty($data['api_delta'])) {
    $info = module_invoke($data['api_module'], 'dibsapi', 'info', $data['api_delta']);
    if (empty($info) || !is_array($info)) {
      drupal_set_message(t('Error redirecting to DIBS: api_module and/or api_delta is not valid!'), 'error');
      return '';
    }
  }
  elseif (!empty($data['api_module']) || !empty($data['api_delta'])) {
    drupal_set_message(t('Error redirecting to DIBS: api_module or api_delta is missing!'), 'error');
    return '';
  }
  elseif ($data['payment_status'] > 0) {
    drupal_set_message(t('Error redirecting to DIBS: the payment has already made!'), 'error');
    return '';
  }

  // Invoking the module that implementet the payment
  module_invoke($data['api_module'], 'dibsapi', 'transaction_cancel', $data['api_delta'], $data);

  // Loading settings
  $settings = dibs_load_settings($data['api_module'], $data['api_delta']);

  // Getting redirect form
  $form = drupal_get_form('dibs_frontend_cancel_form', $settings, $data);

  return theme('dibs_cancel_page', $form, $settings, $data);
}

/**
 * Form builder; Building the cancel form
 *
 * @param array $form_state
 * @param array $settings
 * @param array $data
 *
 * @return array
 *
 * @ingroup forms
 * @see dibs_frontend_cancel_form_submit()
 */
function dibs_frontend_cancel_form($form_state = array(), $settings, $data) {
  $form['hash'] = array('#type' => 'value', '#value' => $data['api_hash']);
  $form['submit'] = array('#type' => 'submit', '#value' => t('Return to DIBS payment'));
  return $form;
}

/**
 * Process the cancel form
 *
 * @param array $form
 * @param array $form_state
 *
 * @ingroup forms
 * @see dibs_frontend_cancel_form()
 */
function dibs_frontend_cancel_form_submit($form, &$form_state) {
  $hash = $form_state['values']['hash'];

  // Loading the transaction
  $data = dibs_load_transaction($hash, 'hash');

  // We are forcing an update of the order id, because edankort requires a new one
  // if the user has cancelled the payment.

  // Generating order id
  $order_id = dibs_get_next_order_id($data['api_module'], $data['api_deta']);

  // Updating transaction
  db_query("UPDATE {dibs_transactions} SET payment_order_id = '%s' WHERE api_hash = '%s'", $order_id, $hash);

  // Redirecting to the payment redirect page
  drupal_goto('payment/dibs/redirect/' . $hash);

  exit;

}

/**
 * DIBS payment ok callback
 *
 * @param string $hash
 */
function dibs_frontend_dibs_callback_ok($hash) {
  if (isset($_REQUEST['transact'])) {
    $transact = $_REQUEST['transact'];
    $authkey = isset($_REQUEST['authkey']) ? $_REQUEST['authkey'] : '';
    $paytype = isset($_REQUEST['paytype']) ? $_REQUEST['paytype'] : '';
    $transaction = dibs_load_transaction($hash, 'hash');
    if (!empty($transaction)) {
      $settings = dibs_load_settings($transaction['api_module'], $transaction['api_delta']);
      $md5_key = md5($settings['general']['md5_key2'] . md5($settings['general']['md5_key1'] . 'transact='. $transact .'&amount='. ($data['payment_price'] * 100) .'&currency='. $settings['general']['currency']));
      if (($md5_key == $authkey) || empty($settings['md5'])) {
        // Updating transaction record with payment transaction id
        db_query("UPDATE {dibs_transactions} SET payment_transaction_id = %d, payment_status = 1, payment_type = '%s' WHERE tid = %d", $transact, $paytype, $transaction['tid']);

        // We are unsetting the transaction and loading it again to get the updated one.
        unset($transaction);
        $transaction = dibs_load_transaction($hash, 'hash');

        // Invoking the module that implementet the payment
        module_invoke($transaction['api_module'], 'dibsapi', 'transaction_callback', $transaction['api_delta'], $transaction);
      }
    }
  }
}

/**
 * DIBS accept page
 *
 * @param string $hash
 */
function dibs_frontend_accept_page($hash) {
  // Loading transaction
  $transaction = dibs_load_transaction($hash, 'hash');

  // Returning an error if the transaction is emtpy
  if (empty($transaction)) {
    drupal_set_message(t('An error occured. The transaction key is not valid!'), 'error');
    return '';
  }

  // Unsetting the temporary session
  unset($_SESSION['dibs']);

  // Invoking the module that implementet the payment
  module_invoke($transaction['api_module'], 'dibsapi', 'transaction_accept', $transaction['api_delta'], $transaction);

  // Loading settings
  $settings = dibs_load_settings($transaction['api_module'], $transaction['api_delta']);

  // Returning the themed output
  return theme('dibs_accept_page', $settings, $transaction);
}


