<?php
// $Id$

/**
 * @file dibs-cancel-page.tpl.php
 *
 * Theme implementation to display the dibs cancel page
 *
 * Available variables:
 * - $form:  The whole form element as a string.
 * - $settings: Full DIBS settings array.
 * - $transaction: Full transaction array with all info about 
 *   the transaction.
 *
 * @see template_preprocess()
 * @see template_preprocess_dibs_cancel_page()
 */
?>
<div id="dibs-cancel-page-<?php print $transaction['api_module']; ?>-<?php print $transaction['api_delta']; ?>" class="dibs-cancel-page clear-block">
  <p><?php print t('You decided to click cancel in the payment form. Please click the button below to return to the payment again.'); ?></p>
  <?php print $form; ?>
</div>
