<?php
// $Id:

/**
 * Makes it possible for other modules to implement the DIBS payment gateway.
 *
 * @param $op
 *   What kind of action is being performed. Possible values:
 *   - "info": Used to get information about the modules that implements the dibsapi.
 *   - "transaction_cancel": Executed when a transaction is cancelled by the user.
 *      For example used to make a module specific redirect after the transaction is
 *      cancelled. 
 *   - "transaction_accept": Executed when a transaction is accepted.
 *      For example used to make a module specific redirect after the transaction is
 *      accepted
 *   - "transaction_callback": Executed after the hidden callback from DIBS when payment
 *      is completed.
 * @param $delta
 *   Which DIBS implementation to return. 
 *   Although it is most commonly an integer starting at 0, this is not mandatory.
 * @param &$transaction
 *   An object with all the information about the transaction.
 * @param $a3
 *  - Not used at the momemt.
 * @param $a4
 *  - Not used at the momemt.
 * @return
 *   This varies depending on the operation.
 *   - The "transaction_cancel", "transaction_accept" and "transaction_callback"
 *     operations have no return value.
 *   - The "info" operation should return an array with info about the DIBS implementations.
 */
function hook_dibsapi($op = 'info', $delta = NULL, &$transaction = NULL, $a3 = NULL, $a4 = NULL) {
  switch ($op) {
    case 'info':
      $info[0] = array('info' => t('DIBS implementation #1.......'));
      $info[1] = array('info' => t('DIBS implementation #2.......'));      
      if (!empty($delta)) {
        return isset($info[$delta]) ? $info[$delta] : NULL;
      }
      else {
        return $info;
      }
      break;            
    case 'transaction_cancel':
      switch($delta) {
        case 0:
          drupal_goto(drupal_get_path_alias('some-path/payment/aborted/1'));
          break;
        case 1:
          drupal_goto(drupal_get_path_alias('some-path/payment/aborted/2'));
          break;          
        }
      break;
    case 'transaction_accept':
      switch($delta) {
        case 0:
          drupal_goto(drupal_get_path_alias('some-path/payment/receipt/1'));
          break;
        case 1:
          drupal_goto(drupal_get_path_alias('some-path/payment/receipt/2'));
          break;          
        }
      break;    
    case 'transaction_callback':
      // Doing some stuff when the payment is completed.
      // For example sending receipt mail or what else is needed
      break;                
  }
}

?>
