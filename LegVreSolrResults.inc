<?php

/*
 * contains methods to search solr and display results.  depends on Apache_Solr_Php client.
 */

module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

/**
 * Extention of IslandoraSolrResults for templating purposes.
 * This overrides the displayResults function to provide an alternate display type.
 */
class LegVreSolrResults extends IslandoraSolrResults {

  function LegVreSolrResults() {
    module_load_include('php', 'islandora_solr_search', 'SolrPhpClient/Apache/Solr/Service');
  }

  /**
   * Outputs results basically in the normal way, but with thumbnails pulled
   * from the Fedora repository.
   *
   * @param $results
   *   A solr resultset object.
   *
   * @return
   *   html output for the resultset. Note: we currently create this
   *   output manually, should refactor to use drupal forms api.
   * This output has been customized for Legislative assembly books
   */
  function printResults($results) {
    drupal_add_css(drupal_get_path('module', 'upei_vre_leg') . '/css/upei_vre_leg.css');
    global $base_url;
    $items = array();
    $type = "ol";
    $title = NULL;
    $recordStart = $results->response->start;
    $limitResults = variable_get('islandora_solr_search_limit_result_fields', 0);
    $rawResponse = $results->getRawResponse();
    $responseArray = json_decode($rawResponse, true);
    $docs = $responseArray['response']['docs'];
    //retrieve 'highlighing' from resultset.
    $highlightResults = $responseArray['highlighting'];
    $highlights = $results->highlighting;

    foreach ($highlights as $highlight) {
      $fields = get_object_vars($highlight);
      $keys = array_keys($fields);
      if (count($keys) > 0) {
        foreach ($highlight->$keys[0] as $body) {
          //drupal_set_message("$keys[0]  $body");
        }
      }
      //drupal_set_message($highlight->$keys[0]);
    }

    global $base_url;
    if (empty($results)) {
      return "no results";
    }
    $q = $responseArray['responseHeader']['params']['q'];
    //we need to get rid of fields and delimiters, the book assumes full text search only
    $terms = preg_split('/"| or | OR | and | AND |[a-z._A-Z]*:|#(?<!\\\)\:#/', $q);
    //q_string should now be only the words searched for in the solr block minus the fields and modifiers
    $q_string = trim(implode(" ", $terms));
    foreach ($results->response->docs as $doc) {
      $rows = array();
      $row = 0;
      $snippet = NULL;
      $sequence_number_variable = 'rels.isSequenceNumber';
      $pid = $doc->PID;
      $cmodel_field = 'rels.hasModel';
      $cmodel = $doc->$cmodel_field;
      if ($cmodel == 'info:fedora/islandora:biography_image_CModel') {
        $link_pid = $doc->PID;
        $link_title = $doc->title;
        $l = l($link_title, 'fedora/repository/' . htmlspecialchars($link_pid, ENT_QUOTES, 'utf-8'));
      }
      else {
        $link_pid = isset($doc->PARENT_pid_s) ? $doc->PARENT_pid_s : $doc->PID;
        $link_title = isset($doc->title) ? $doc->title : $doc->PID;
        $l = l($link_title, 'fedora/repository/' . htmlspecialchars($link_pid, ENT_QUOTES, 'utf-8'), array('query' => array('startpage' => $doc->{"$sequence_number_variable"}, 'solrq' => $q_string)));
      }


      $rows[$row][] = array('data' => l("<img src = '$base_url/fedora/repository/" . $pid . "/TN' class='upei_vre_leg_align-left upei_vre_leg_frame'/>", 'fedora/repository/' . htmlspecialchars($link_pid, ENT_QUOTES, 'utf-8'), array('query' => array('startpage' => $doc->{"$sequence_number_variable"}, 'solrq' => $q_string), 'html' => TRUE)),
        'rowspan' => 8);

      $rows[$row][] = array(
        'data' => 'Title:',
        'header' => TRUE,
      );
      $rows[$row++][] = array(
        'data' => $l,
      );
      if (isset($doc->mods_affiliation)) {
        $rows[$row][] = array(
          'data' => 'Affiliation(s):',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' =>  $doc->mods_affiliation,
        );
      }
      if (isset($doc->region)) {
        $rows[$row][] = array(
          'data' => 'Riding(s):',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' =>  $doc->region,
        );
      }
      if (isset($doc->mods_occupation)) {
        $rows[$row][] = array(
          'data' => 'Occupation(s):',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' =>  $doc->mods_occupation,
        );
      }
       if (isset($doc->mods_role)) {
        $rows[$row][] = array(
          'data' => 'Office(s):',
          'header' => TRUE,
        );
        $offices = $doc->mods_role;        
        if(is_array($offices)){
          $offices = implode(', ', $offices);
        }
        $rows[$row++][] = array(
          'data' =>  $offices,
        );
      }
      //PARENT_dateIssued_s
      if ($doc->PARENT_dateIssued_s) {
        $rows[$row][] = array(
          'data' => 'Date:',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' => $doc->PARENT_dateIssued_s,
        );
      }
      if (isset($doc->ocr_text)) {
        $rows[$row][] = array(
          'data' => 'Snippet:',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' => $doc->ocr_text,
        );
      }
      if (!empty($doc->PARENT_session_s)) {
        $rows[$row][] = array(
          'data' => 'Source:',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' => 'Part of ' . (string) 'Session ' . $doc->PARENT_session_s . ', Assembly ' . (string) $doc->PARENT_assembly_s,
        );
      }
      if (isset($doc->PARENT_genre_s)) {
        $rows[$row][] = array(
          'data' => 'Type:',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' => $doc->PARENT_genre_s,
        );
      }

      if (isset($doc->PARENT_topis_s)) {
        $rows[$row][] = array(
          'data' => 'Topic:',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' => $doc->PARENT_topic_s,
        );
      }

      if (isset($doc->PARENT_subject_s)) {
        $rows[$row][] = array(
          'data' => 'Subject:',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' => $doc->PARENT_subject_s,
        );
      }
      if (isset($doc->subject)) {
        $rows[$row][] = array(
          'data' => 'Subject(s):',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' => implode(', ', $doc->subject),
        );
      }
      if (isset($doc->coverage)) {
        $rows[$row][] = array(
          'data' => 'Information:',
          'header' => TRUE,
        );
        $rows[$row++][] = array(
          'data' => implode(', ', $doc->coverage),
        );
      }
      
      if (is_array($keys)) {
        $snippet = $highlightResults[$value][$keys[0]];
      }

      if ($snippet) {
        $rows[$row][] = array(
          'data' => 'Full Text',
          'header' => TRUE,
        );
        $rows[$row][] = $snippet[0];
      }
      //$tnString = "<img src = '$base_url/fedora/repository/" . $pid . "/TN' class='upei_vre_leg_align-left upei_vre_leg_frame'/>";
      $items[] = theme_table(NULL, $rows, array('class' => 'islandora_solr_search_results_object'));
    }

    $output = theme_item_list($items, $title, $type, array('class' => 'stuff', 'start' => ($recordStart + 1)));
    if (variable_get('islandora_solr_search_debug_mode', 0)) { // debug dump
      $results_r .= "<pre>Results: " . print_r($results, TRUE) . "</pre>";
      $fieldset_r = array(
        '#title' => t("Raw Results"),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#value' => $results_r,
      );
      $output .= theme('fieldset', $fieldset_r);
    }
    return $output;
  }

}

