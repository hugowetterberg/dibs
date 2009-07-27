<?php
// $Id$

/**
 * @file
 * Admin functions for the DIBS module
 */

 /**
 * This function is called when user enter in admin/dibs
 * @return string
 */
function dibs_admin_overview() {
  $output = '';
  $item = menu_get_item('admin/dibs');
  $content = system_admin_menu_block($item);
  $output = theme('admin_block_content', $content);
  return $output;
}

/**
 * List of implementations of the DIBS module.
 * Link to edit settings for each implementation
 * @return string
 */
function dibs_admin_settings_list() {
  drupal_add_css(drupal_get_path('module', 'dibs') .'/dibs.admin.css');

  // Looping through all modules that implement the hook_dibsapi.
  $output = '<br />';
  $i = 0;
  foreach ((array)module_implements('dibsapi') as $module) {
    if ($i++ > 0) {
      $output .= '<p></p>';
    }
    $output .= '<h3>' . t('Module') . ': ' . $module . '</h3>';

    // Adding table headers
    $header = array(
      array('data' => t('Name'), 'class' => 'dibs-admin-settings-title'),
      array('data' => t('Operation'), 'class' => 'dibs-admin-settings-operation')
    );

    $rows = array();
    // Looping through deltas for a specific module implementation of hook_dibsapi)).
    foreach ((array)module_invoke($module, 'dibsapi') as $delta => $info) {
      $row = array(
        array('data' => $info['info'], 'class' => ''),
        array('data' => l(t('edit'), 'admin/dibs/settings/edit/' . $module . '/' . $delta), 'class' => ''),
      );
      $rows[] = $row;
    }
    $output .= theme('table', $header, $rows, array('class' => 'dibs-admin-settings-list'));
  }

  return $output;
}

/**
 * Form builder; The settings edit form
 *
 * @param array $form_state
 * @param string $module
 * @param multi $delta
 *
 * @ingroup forms
 * @see dibs_admin_settings_form_validate()
 * @see dibs_admin_settings_form_submit()
 */
function dibs_admin_settings_form($form_state = array(), $module = NULL, $delta = NULL) {
  // Checking if the module and delta exists
  $info = module_invoke($module, 'dibsapi', 'info', $delta);
  if (empty($info) || !is_array($info)) {
    drupal_not_found();
    exit;
  }

  // Checking if module and delta are default
  $default = dibs_is_default($module, $delta);

  // Setting page title
  drupal_set_title(t('Change DIBS implementation settings: %module > %delta', array('%module' => t($module), '%delta' => $info['info'])));

  // Loading settings
  $settings = dibs_load_settings($module, $delta, FALSE);

  $form = array();
  // General
  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General DIBS settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );
  $form['general']['merchant'] = array(
    '#type' => 'textfield',
    '#title' => t('MertchantID'),
    '#maxlength' => 30,
    '#required' => $default,
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'merchant', ''),
    '#description' => t('DIBS MerchantID')
  );
  $form['general']['account'] = array(
    '#type' => 'textfield',
    '#title' => t('Account'),
    '#maxlength' => 64,
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'account', ''),
    '#description' => t('DIBS Account ID. Only used if the DIBS gateway is running multiple accounts.')
  );
  $form['general']['test_mode'] = array(
    '#type' => 'select',
    '#title' => t('Test mode'),
    '#options' => array('' => '', 0 => t('No'), 1 => t('Yes')),
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'test_mode', ''),
    '#description' => t('Is the gateway running in test mode'),
  );
  $form['general']['type'] = array(
    '#type' => 'select',
    '#title' => t('Window type'),
    '#required' => $default,
    '#options' => array('' => '', 'pay' => t('Pay window'), 'flex' => t('Flex window')),
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'type', ($default ? 1 : '')),
    '#description' => t('If enabled, DIBS will make some extra checks on the sended data, to be sure that no one manipulated it. If enabled should the keys below be filled in!'),
  );
  $form['general']['retry_handling'] = array(
    '#type' => 'select',
    '#title' => t('Order id handling after cancel'),
    '#options' => array('' => '', 'new_order_id' => t('Generate a new order id'), 'add_retry_suffix' => t('Add retry suffix'), 'unchanged' => t('Preserve order id')),
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'retry_handling', ($default ? 'new_order_id' : '')),
    '#description' => t('How the order id should be handled when the user retries a cancelled payment. Some card providers (edankort) require a new order ID after cancellation.'),
  );
  $form['general']['md5'] = array(
    '#type' => 'select',
    '#title' => t('Perform MD5 control'),
    '#options' => array('' => '', 0 => t('No'), 1 => t('Yes')),
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'md5', ($default ? 1 : '')),
    '#description' => t('If enabled, DIBS will make some extra checks on the sended data, to be sure that no one manipulated it. If enabled should the keys below be filled in!'),
  );
  $form['general']['md5_key1'] = array(
    '#type' => 'textfield',
    '#title' => t('MD5 key 1'),
    '#maxlength' => 64,
    '#required' => FALSE,
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'md5_key1', ''),
    '#description' => t('MD5 key 1 - used if "Perform MD5 control" is enabled.')
  );
  $form['general']['md5_key2'] = array(
    '#type' => 'textfield',
    '#title' => t('MD5 key 2'),
    '#maxlength' => 64,
    '#required' => FALSE,
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'md5_key2', ''),
    '#description' => t('MD5 key 2 - used if "Perform MD5 control" is enabled.')
  );
  $form['general']['lang'] = array(
    '#type' => 'select',
    '#title' => t('Language'),
    '#options' => array(
                        '' => '',
                        'da' => 'Danish',
                        'sv' => 'Swedish',
                        'no' => 'Norwegian',
                        'en' => 'English',
                        'nl' => 'Dutch',
                        'de' => 'German',
                        'fr' => 'French',
                        'fi' => 'Finnish',
                        'es' => 'Spanish',
                        'it' => 'Italian',
                        'pl' => 'Polish'
                      ),
    '#required' => $default,
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'lang', ''),
    '#description' => t('Language code for the language used on the DIBS payment window')
  );
  $form['general']['currency'] = array(
    '#type' => 'select',
    '#title' => t('Currency'),
    '#options' => array(
                        '' => '',
                        '208' => 'Danish Kroner (DKK)',
                        '978' => 'Euro (EUR)',
                        '840' => 'US Dollar $ (USD)',
                        '826' => 'English Pound £ (GBP)',
                        '752' => 'Swedish Kronor (SEK)',
                        '036' => 'Australian Dollar (AUD',
                        '124' => 'Canadian Dollar (CAD)',
                        '352' => 'Icelandic Króna (ISK)',
                        '392' => 'Japanese Yen (JPY)',
                        '554' => 'New Zealand Dollar (NZD)',
                        '578' => 'Norwegian Kroner (NOK)',
                        '756' => 'Swiss Franc (CHF)',
                        '949' => 'Turkish Lire (TRY)',
                      ),
    '#required' => $default,
    '#default_value' => dibs_get_settings_value($module, $delta, 'general', 'currency', ''),
    '#description' => t('Currency code for the currency used when paying.')
  );

  // Payment window
  $form['paymentwindow'] = array(
    '#type' => 'fieldset',
    '#title' => t('Payment Window settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => TRUE,
  );
  $form['paymentwindow']['color'] = array(
    '#type' => 'select',
    '#title' => t('Color theme'),
    '#options' => array(
                        '' => '',
                        'sand' => t('Sand'),
                        'grey' => t('Grey'),
                        'blue' => t('Blue'),
                      ),
    '#required' => FALSE,
    '#default_value' => dibs_get_settings_value($module, $delta, 'paymentwindow', 'color', ''),
    '#description' => t('The color theme for the DIBS popup window.')
  );

  // Flex window
  $form['flexwindow'] = array(
    '#type' => 'fieldset',
    '#title' => t('Flex Window settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => TRUE,
  );
  $form['flexwindow']['color'] = array(
    '#type' => 'select',
    '#title' => t('Color theme'),
    '#options' => array(
                        '' => '',
                        'sand' => t('Sand'),
                        'grey' => t('Grey'),
                        'blue' => t('Blue'),
                      ),
    '#required' => FALSE,
    '#default_value' => dibs_get_settings_value($module, $delta, 'flexwindow', 'color', ''),
    '#description' => t('The color theme for the DIBS popup window.')
  );
  $form['flexwindow']['decorator'] = array(
    '#type' => 'select',
    '#title' => t('Decorator'),
    '#options' => array(
                        '' => '',
                        'default' => t('Default'),
                        'basal' => t('Basal'),
                        'rich' => t('Rich'),
                        'custom' => t('Custom'),
                      ),
    '#required' => FALSE,
    '#default_value' => dibs_get_settings_value($module, $delta, 'flexwindow', 'decorator', ''),
    '#description' => t('Choose what DIBS decorator to use. If you want to use the one configured in the DIBS administration, please then choose "Custom".')
  );
  $form['flexwindow']['voucher'] = array(
    '#type' => 'select',
    '#title' => t('Voucher'),
    '#options' => array(
                        '' => '',
                        0 => t('No'),
                        1 => t('Yes'),
                      ),
    '#required' => FALSE,
    '#default_value' => dibs_get_settings_value($module, $delta, 'flexwindow', 'voucher', ''),
    '#description' => t('If set to Yes, then the list of payment types on the first page of FlexWin will contain vouchers, too.')
  );

  // Order ID
  $form['orderid'] = array(
    '#type' => 'fieldset',
    '#title' => t('Order ID settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => TRUE,
  );
  $form['orderid']['order_id_old'] = array(
    '#type' => 'value',
    '#value' => !empty($settings['orderid']['order_id']) ? $settings['orderid']['order_id'] : '',
  );
  $form['orderid']['order_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Next order ID'),
    '#maxlength' => 10,
    '#legnth' => 10,
    '#required' => FALSE,
    '#default_value' => !empty($settings['orderid']['order_id']) ? $settings['orderid']['order_id'] : '',
    '#description' => t('The next order ID value. Only numeric values are allowed. Please besure that the range used does not conflict with order IDs on existing payments in DIBS, and other implementations of the DIBS module.')
  );
  $form['orderid']['order_id_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Order ID prefix'),
    '#maxlength' => 10,
    '#legnth' => 10,
    '#required' => FALSE,
    '#default_value' => !empty($settings['orderid']['order_id_prefix']) ? $settings['orderid']['order_id_prefix'] : '',
    '#description' => t('Prefix used before the order ID value, when the payment is sent to DIBS. Using the prefix and/or suffix makes it easier to control that the order ID is unique when sent to DIBS.')
  );
  $form['orderid']['order_id_suffix'] = array(
    '#type' => 'textfield',
    '#title' => t('Order ID suffix'),
    '#maxlength' => 10,
    '#legnth' => 10,
    '#required' => FALSE,
    '#default_value' => !empty($settings['orderid']['order_id_suffix']) ? $settings['orderid']['order_id_suffix'] : '',
    '#description' => t('Suffix used after the order ID value, when the payment is sent to DIBS. Using the prefix and/or suffix makes it easier to control that the order ID is unique when sent to DIBS.')
  );

  // Advanced settings
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => TRUE,
  );
  $form['advanced']['calcfee'] = array(
    '#type' => 'select',
    '#title' => t('Calc fee'),
    '#options' => array('' => '', 0 => t('No'), 1 => t('Yes')),
    '#default_value' => dibs_get_settings_value($module, $delta, 'advanced', 'calcfee', ($default ? 0 : '')),
    '#description' => t('If enabled, the Payment Window will automatically affix the charge due to the transaction, i.e. the charge payable to the acquirer (e.g. PBS), and display this to the customer.'),
  );
  $form['advanced']['capturenow'] = array(
    '#type' => 'select',
    '#title' => t('Capture now'),
    '#options' => array('' => '', 0 => t('No'), 1 => t('Yes')),
    '#default_value' => dibs_get_settings_value($module, $delta, 'advanced', 'capturenow', ($default ? 0 : '')),
    '#description' => t('If enabled, the amount is immediately transferred from the customers account to the shop\'s account. This function can only be utilized in the event that there is no actual physical delivery of any items.'),
  );
  $form['advanced']['unqiue_order_id'] = array(
    '#type' => 'select',
    '#title' => t('Unique order ID'),
    '#options' => array('' => '', 0 => t('No'), 1 => t('Yes')),
    '#default_value' => dibs_get_settings_value($module, $delta, 'advanced', 'unqiue_order_id', ($default ? 1 : '')),
    '#description' => t('If enabled, the order ID must be unique, i.e. there is no existing transaction with DIBS with the same order ID. If such a transaction already exists, payment will be rejected with reason=7.'),
  );

  $form['module'] = array(
    '#type' => 'value',
    '#value' => $module,
  );
  $form['delta'] = array(
    '#type' => 'value',
    '#value' => $delta,
  );
  $form['submit'] = array('#type' => 'submit', '#value' => t('Save settings'));

  return $form;
}

/**
 * Validate settings form
 *
 * @param array $form
 * @param array $form_state
 */
function dibs_admin_settings_form_validate($form, &$form_state) {
  $module = $form_state['values']['module'];
  $delta  = $form_state['values']['delta'];

  // Checking if the module and delta exists
  $info = module_invoke($module, 'dibsapi', 'info', $delta);
  if (empty($info) || !is_array($info) || empty($module) || empty($delta)) {
    drupal_not_found();
    exit;
  }

  // Checking if module and delta are default
  $default = dibs_is_default($module, $delta);

  // Checking if keys are filled in when MD5 check is enabled
  if ($form_state['values']['general']['md5'] == 1) {
    if (empty($form_state['values']['general']['md5_key1'])) {
      form_set_error('general][md5_key1', t('Please, fill in the missing key, or disable "Perform MD5 control". The MD5 keys can be found in the DIBS administration.'));
    }
    if (empty($form_state['values']['general']['md5_key2'])) {
      form_set_error('general][md5_key2', t('Please, fill in the missing key, or disable "Perform MD5 control". The MD5 keys can be found in the DIBS administration.'));
    }
  }

  // Checking order ID settings
  if ($default && empty($form_state['values']['orderid']['order_id'])) {
    form_set_error('orderid][order_id', t('Please, fill in the next order ID.'));
  }
  elseif (!empty($form_state['values']['orderid']['order_id']) && !is_numeric($form_state['values']['orderid']['order_id'])) {
    form_set_error('orderid][order_id', t('The order ID should be numeric.'));
  }

}

/**
 * Process the settings form submit
 *
 * @param array $form
 * @param array $form_state
 */
function dibs_admin_settings_form_submit($form, &$form_state) {
  $module         = $form_state['values']['module'];
  $delta          = $form_state['values']['delta'];
  $general        = $form_state['values']['general'];
  $paymentwindow  = $form_state['values']['paymentwindow'];
  $flexwindow     = $form_state['values']['flexwindow'];
  $orderid        = $form_state['values']['orderid'];
  $advanced       = $form_state['values']['advanced'];

  // Checking if the module and delta exists
  $info = module_invoke($module, 'dibsapi', 'info', $delta);
  if (empty($info) || !is_array($info) || empty($module) || empty($delta)) {
    drupal_not_found();
    exit;
  }

  // Creating array with settings and saving it as a variable
  $settings = array(
    'general' => $general,
    'paymentwindow' => $paymentwindow,
    'flexwindow' => $flexwindow,
    'advanced' => $advanced
  );
  variable_set('dibs_settings_' . $module . '_' . $delta, $settings);

  // Saving order ID settings
  $result = db_result(db_query("SELECT api_module FROM {dibs_order_id_settings} WHERE api_module = '%s' AND api_delta = '%s'", $module, $delta));

  if (!empty($result)) {
    db_query("UPDATE {dibs_order_id_settings} SET order_id = %d, order_id_suffix = '%s', order_id_prefix = '%s' WHERE api_module = '%s' AND api_delta = '%s'",
      $orderid['order_id'], $orderid['order_id_suffix'], $orderid['order_id_prefix'], $module, $delta);
  }
  else {
    db_query("INSERT INTO {dibs_order_id_settings} (api_module, api_delta, order_id, order_id_prefix, order_id_suffix) VALUES ('%s', '%s', %d, '%s', '%s')",
      $module, $delta, $orderid['order_id'], $orderid['order_id_prefix'], $orderid['order_id_suffix']);
  }

  drupal_get_messages();
  drupal_set_message(t('The settings are now saved.'));

  $form_state['redirect'] = 'admin/dibs/settings';
  return;
}
