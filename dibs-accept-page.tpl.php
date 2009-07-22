<?php
// $Id$

/**
 * @file dibs-accept-page.tpl.php
 *
 * Theme implementation to display the dibs accept page
 *
 * Available variables:
 * - $settings: Full DIBS settings array.
 * - $transaction: Full transaction array with all info about 
 *   the transaction.
 *
 * @see template_preprocess()
 * @see template_preprocess_dibs_accept_page()
 */
?>
<div id="dibs-accept-page-<?php print $transaction['api_module']; ?>-<?php print $transaction['api_delta']; ?>" class="dibs-accept-page clear-block">
  <p><?php print t('We have now received your payment. Thanks for the payment.'); ?></p>
</div>
