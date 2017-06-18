<?php

/**
 * @file
 * Cablecast API
 */

/**
 * Cablecast server query
 *
 * @param $method
 * @param array $data
 * @return mixed
 */
function cablecast_api_server_query($method, $data) {
  //dsm($data, "Soap method: $method");
  try {
    $client = cablecast_api_get_client();
    //add username and password of $data
    $data['username'] = variable_get('cablecast_api_server_username', NULL);
    $data['password'] = variable_get('cablecast_api_server_password', NULL);
    // perform query
    $return = $client->$method($data);

    return $return;

  } // end try

  catch (SoapFault $fault) {
    $data_message = "Method: $method -- ";
    foreach ($data as $key=>$value) {
      if ($key != 'username' && $key != 'password') {
	$data_message .= "$key=$value || ";
      }
    }

    $error_message = 
      "The following SOAP Error was generated, ".
      "SOAP Fault: (faultcode: {".$fault->faultcode."}, ".
      "faultstring: {$fault->faultstring}) ".
      "data: $data_message";

    trigger_error($error_message, E_USER_ERROR);

    if (user_access('administer cablecast')) {
      drupal_set_message($error_message, 'error');
    }
  }// end catch
}

/**
 * Creates a new Producer on Cablecast
 *
 * @param $data
 *
 * @return $result
 */
function cablecast_api_add_producer($user) {
  /*
      <LocationID>int</LocationID>
      <ProducerName>string</ProducerName>
      <Phone1>string</Phone1>
      <Phone2>string</Phone2>
      <Address>string</Address>
      <Notes>string</Notes>
      <Active>boolean</Active>
      <ContactName>string</ContactName>
      <Email>string</Email>
      <Website>string</Website>
      <username>string</username>
      <password>string</password>
  */
  $name = $user->display_name ? $user->display_name : $user->name;

   // setup data array for SOAP call
  $data = array(
    'LocationID'    => 1,
    'ProducerName'  => $name,
    'Phone1'        => '',
    'Phone2'        => '',
    'Address'       => '',
    'Notes'         => '',
    'Active'        => 1,
    'ContactName'   => $name,
    'Email'         => $user->mail,
    'Website'       => '',
  );

  $producer_id = cablecast_api_server_query('CreateNewProducer', $data);

  return $producer_id->CreateNewProducerResult;

}

/**
 * Updates a Producer on Cablecast
 *
 * @param $data
 * @return $result
 */
function cablecast_api_update_producer($user) {
  /*
      <ProducerID>int</ProducerID>
      <ProducerName>string</ProducerName>
      <Phone1>string</Phone1>
      <Phone2>string</Phone2>
      <Address>string</Address>
      <Notes>string</Notes>
      <Active>boolean</Active>
      <ContactName>string</ContactName>
      <Email>string</Email>
      <Website>string</Website>
      <username>string</username>
      <password>string</password>
  */

  $cc_producer_id = 
    cablecast_api_get_single_field_value($user, 'field_cablecast_producer_id',
					 'value');

  if (!$cc_producer_id) {
    return NULL;
  }

  $name = (isset($user->display_name) && $user->display_name)
    ? $user->display_name : $user->name;

  // setup data array for SOAP call
  $data = array(
		'ProducerID'    => $cc_producer_id,    
		'ProducerName'   => $name,
		'Phone1'        => '',
		'Phone2'        => '',
		'Address'       => '',
		'Notes'         => '',
		'Active'        => 1,
		'ContactName'   => $name,
		'Email'         => $user->mail,
		'Website'       => '',
		);

  $producer_id = cablecast_api_server_query('UpdateProducer', $data);

  return $producer_id->UpdateProducerResult;
}

/**
 * Retrieve a Producer from Cablecast
 *
 * @param $data
 * @return $return
 */
function cablecast_api_get_producer($user) {
  /*
      <ProducerID>int</ProducerID>
      <username>string</username>
      <password>string</password>
  */

  // setup data array for SOAP call
  $data = array(
    'ProducerID'    => $user->field_cablecast_producer_id[LANGUAGE_NONE][0]['value'],
  );

  $producer_id = cablecast_api_server_query('GetProducerInfo', $data);

  return $producer_id->GetProducerInfoResult->ProducerID;

}


/**
 * Retrieve a Producer from Cablecast
 *
 * @param $data
 * @return $return
 */
function cablecast_api_get_producer_object($id) {
  /*
      <ProducerID>int</ProducerID>
      <username>string</username>
      <password>string</password>
  */

  // setup data array for SOAP call
  $data = array(
		'ProducerID'    => $id,
  );

  $producer_id = cablecast_api_server_query('GetProducerInfo', $data);

  return $producer_id->GetProducerInfoResult;

}

/**
 * Retrieve a Producer ID from Cablecast
 *
 * @param $data
 * @return $return
 */
function cablecast_api_get_producer_from_id($uid) {
  /*
      <ProducerID>int</ProducerID>
      <username>string</username>
      <password>string</password>
  */

  // setup data array for SOAP call
  $data = array(
    'ProducerID'    => $uid,
  );

  $producer = cablecast_api_server_query('GetProducerInfo', $data);

  return $producer->GetProducerInfoResult->ProducerID;

}

/**
 * Create a Project on Cablecast
 *
 * @param $data
 * @return $return
 */
function cablecast_api_add_project($node) {
 /*
   <LocationID>int</LocationID>
   <Name>string</Name>
   <Description>string</Description>
   <ProducerID>int</ProducerID>
   <Podcast>boolean</Podcast>
   <PodcastName>string</PodcastName>
   <PodcastDescription>string</PodcastDescription>
   <username>string</username>
   <password>string</password>
 */

  // fetch the cablecast producer id
  $d_producer = $node->uid ? user_load($node->uid) : NULL;

  $cc_producer_id = $d_producer ?
    cablecast_api_get_single_field_value($d_producer, 
					 'field_cablecast_producer_id',
					 'value') : 0;
  

  // setup data array for SOAP call
  $data = array(
    'LocationID'          => 1,
    'Name'                => check_plain($node->title),
    'Description'         => '',
    'ProducerID'          => $cc_producer_id,
    'Podcast'             => FALSE,
    'PodcastName'         => '',
    'PodcastDescription'  => '',
  );

  $project_id = cablecast_api_server_query('CreateNewProject', $data);

  return $project_id->CreateNewProjectResult;
}

/**
 * Updates a Project on Cablecast
 *
 * @param $data
 * @return $return
 */
function cablecast_api_update_project($node) {
  /*
    <ProjectID>int</ProjectID>
    <Name>string</Name>
    <Description>string</Description>
    <ProducerID>int</ProducerID>
    <Podcast>boolean</Podcast>
    <PodcastName>string</PodcastName>
    <PodcastDescription>string</PodcastDescription>
    <username>string</username>
    <password>string</password>
  */
  // fetch the cablecast project id
  $cc_project_id = 
    cablecast_api_get_single_field_value($node, 'field_cablecast_project_id');

  if (!$cc_project_id) {
    return NULL;
  }

  // fetch the cablecast producer id
  $d_producer = $node->uid ? user_load($node->uid) : NULL;

  $cc_producer_id = $d_producer ?
    cablecast_api_get_single_field_value($d_producer, 
					 'field_cablecast_producer_id',
					 'value') : 0;

  // setup data array for SOAP call
  $data = array(
		'ProjectID'           => $cc_project_id,
		'Name'                => check_plain($node->title),
		'Description'         => '',
		'ProducerID'          => $cc_producer_id,
		'Podcast'             => FALSE,
		'PodcastName'         => '',
		'PodcastDescription'  => '',
		);
  $project_id = cablecast_api_server_query('UpdateProject', $data);

  return $project_id->UpdateProjectResult;
}

/**
 * Retrieves all Projects from Cablecast
 * Cablecast lacks a method to get a single Project
 *
 * @param $data
 *
 * @return bool $return
 */
function cablecast_api_get_projects($node) {
  /*
    <ChannelID>int</ChannelID>
  */

  $project_id = $node->field_cablecast_project_id[LANGUAGE_NONE][0]['value'];

  // setup data array for SOAP call
  $data = array(
    'ChannelID' => 1,
  );

  $projects = cablecast_api_server_query('GetProjects', $data);

  // return project_id if the Project is found
  foreach ($projects->GetProjectsResult->Project as $project) {
    if ($project->ProjectID == $project_id) {
      return $project_id;
    }
  }

  return FALSE;
}

/**
 * Retrieves all Projects from Cablecast
 * Cablecast lacks a method to get a single Project
 *
 * @param $data
 *
 * @return bool $return
 */
function cablecast_api_get_project_from_id($project_id) {
  /*
    <ChannelID>int</ChannelID>
  */

  // setup data array for SOAP call
  $data = array(
    'ChannelID' => 1,
  );

  $projects = cablecast_api_server_query('GetProjects', $data);

  // return project_id if the Project is found
  foreach ($projects->GetProjectsResult->Project as $project) {
    if ($project->ProjectID == $project_id) {
      return $project_id;
    }
  }

  return FALSE;
}


/**
 * Finds the cablecast Project object by retrieving all Projects from Cablecast
 * and looping until a id match. There is no direct fetch of a project in the 
 * cablecast api.
 *
 * @param $data
 *
 * @return bool $return
 */
function cablecast_api_get_project($project_id) {
  /*
    <ChannelID>int</ChannelID>
  */

  // setup data array for SOAP call
  $data = array(
    'ChannelID' => 1,
  );

  $projects = cablecast_api_server_query('GetProjects', $data);

  // return project_id if the Project is found
  foreach ($projects->GetProjectsResult->Project as $project) {
    if ($project->ProjectID == $project_id) {
      return $project;
    }
  }

  return FALSE;
}

/**
 *
 * Adds a Show to Cablecast
 *
 * @param $data
 * @return $return
 */
function cablecast_api_add_show($node) {
  /*
      <LocalID>string</LocalID>
      <LocationID>int</LocationID>
      <Title>string</Title>
      <CGTitle>string</CGTitle>
      <ProjectID>int</ProjectID>
      <CGExempt>boolean</CGExempt>
      <ProducerID>int</ProducerID>
      <Reels>
        <NewReel>
          <FormatID>int</FormatID>
          <CueSeconds>int</CueSeconds>
          <LengthSeconds>int</LengthSeconds>
          <Title>int</Title>
          <Chapter>int</Chapter>
        </NewReel>
        <NewReel>
          <FormatID>int</FormatID>
          <CueSeconds>int</CueSeconds>
          <LengthSeconds>int</LengthSeconds>
          <Title>int</Title>
          <Chapter>int</Chapter>
        </NewReel>
      </Reels>
      <CategoryID>int</CategoryID>
      <EventDate>dateTime</EventDate>
      <Comments>string</Comments>
      <CustomFields>
        <CustomField>
          <Name>string</Name>
          <Value>string</Value>
        </CustomField>
        <CustomField>
          <Name>string</Name>
          <Value>string</Value>
        </CustomField>
      </CustomFields>
      <OnDemand>boolean</OnDemand>
      <OnDemandStatus>string</OnDemandStatus>
      <BugText>string</BugText>
      <CrawlText>string</CrawlText>
      <CrawlLengthInSeconds>int</CrawlLengthInSeconds>
      <username>string</username>
      <password>string</password>
   */
  
  // fetch the cablecast project id
  $d_project_id = 
    cablecast_api_get_single_field_value($node, 'og_group_ref',
					 'target_id');
  $d_project = $d_project_id ? node_load($d_project_id) : NULL;

  $cc_project_id = $d_project ?
    cablecast_api_get_single_field_value($d_project, 
					 'field_cablecast_project_id',
					 'value') : 0;

  // fetch the cablecast producer id
  $d_producer = $d_project ? user_load($d_project->uid):user_load($node->uid);
  $cc_producer_id = $d_producer ?
    cablecast_api_get_single_field_value($d_producer, 
					 'field_cablecast_producer_id',
					 'value') : 0;
  

  $length = cablecast_api_get_single_field_value($node, 'field_runtime',
						 'value');
  $length = $length ? $length : 1;
      

  // All Cablecast shows need at least 1 Reel
  $reels = new stdClass();
  $reels->NewReel = new stdClass();
  $reels->NewReel->FormatID = 1;
  $reels->NewReel->CueSeconds = 0;
  $reels->NewReel->LengthSeconds = $length;
  $reels->NewReel->Title = 1;
  $reels->NewReel->Chapter = 1;

  // setup data array for SOAP call
  $data = array(
    'LocalID'               => "Added by API",
    'LocationID'            => 1,
    'Title'                 => check_plain($node->title),
    'CGTitle'               => check_plain($node->title),
    'ProjectID'             => $cc_project_id,
    // TODO: may vary based on project type or other criteria
    'CGExempt'              => FALSE, 
    'ProducerID'            => $cc_producer_id,
    'Reels'                 => $reels,
    'CategoryID'            => 1,
    'EventDate'             => $time = date("c", time()),
    'Comments'              => '',
    'OnDemand'              => FALSE,
    'OnDemandStatus'        => FALSE,
    'BugText'               => '',
    'CrawlText'             => '',
    'CrawlLengthInSeconds'  => '',
    'CustomFields'          => array(),
  );

  drupal_alter('cablecast_show_presave', $node, $d_project, $data);

  $show_id = cablecast_api_server_query('CreateNewShowRecord', $data);

  return $show_id->CreateNewShowRecordResult;
}

/**
 * Creates a Show record on Cablecast
 *
 * Updates an existing Show Record. All fields are required.
 * Blank or empty fields will overwrite existing show data.
 *
 * @param $data
 * @return $return
 */
function cablecast_api_update_show($node) {
  /*
    <ShowID>int</ShowID>
    <LocalID>string</LocalID>
    <Title>string</Title>
    <CGTitle>string</CGTitle>
    <ProjectID>int</ProjectID>
    <CGExempt>boolean</CGExempt>
    <ProducerID>int</ProducerID>
    <CategoryID>int</CategoryID>
    <EventDate>dateTime</EventDate>
    <Comments>string</Comments>
    <OnDemand>boolean</OnDemand>
    <OnDemandStatus>string</OnDemandStatus>
    <BugText>string</BugText>
    <CrawlText>string</CrawlText>
    <CrawlLengthInSeconds>int</CrawlLengthInSeconds>
    <username>string</username>
    <password>string</password>
   */

  $cc_show_id = $node->field_cablecast_show_id[LANGUAGE_NONE][0]['value'];

  $cc_show = $cc_show_id ?
    cablecast_api_server_query('GetShowInformation',
			       array('ShowID'=>
				       $cc_show_id)) : NULL;
  $cc_show = ($cc_show && isset($cc_show->GetShowInformationResult)) ?
    $cc_show->GetShowInformationResult : NULL;

  // fetch the cablecast project id
  $d_project_id = 
    cablecast_api_get_single_field_value($node, 'og_group_ref',
					 'target_id');
  $d_project = $d_project_id ? node_load($d_project_id) : NULL;

  $cc_project_id = $d_project ?
    cablecast_api_get_single_field_value($d_project, 
					 'field_cablecast_project_id',
					 'value') : 0;

  // fetch the cablecast producer id
  $d_producer = $d_project ? user_load($d_project->uid) : 
    user_load($node->uid);
  $cc_producer_id = $d_producer ?
    cablecast_api_get_single_field_value($d_producer, 
					 'field_cablecast_producer_id',
					 'value') : 0;
  

  // FIXME, ADD ALL CUSTOM DATA FIELDS TO A HOOKABLE FUNCTION SO WE DON'T HAVE
  // PCM SPECIFIC STUFF
  $length = cablecast_api_get_single_field_value($node, 'field_runtime',
						 'value');
  $length = $length ? $length : 1;

  $custom_fields = array();

  // setup data array for SOAP call
  $data = array(
		'ShowID'                => $cc_show_id,
		'LocalID'               => "Added by API",
		'LocationID'            => 1,
		'Title'                 => check_plain($node->title),
		'CGTitle'               => check_plain($node->title),
		'ProjectID'             => $cc_project_id,
		// TODO: may vary based on project type or other criteria
		'CGExempt'              => FALSE, 
		'ProducerID'            => $cc_producer_id,
		'CategoryID'            => 1,
		'EventDate'             => date("c", $node->created),
		'Comments'              => $cc_show->Comments,
		'OnDemand'              => FALSE,
		'OnDemandStatus'        => FALSE,
		'BugText'               => '',
		'CrawlText'             => '',
		'CrawlLengthInSeconds'  => '',
		'TotalSeconds'           => $length,
		'CustomFields'          => $custom_fields,
		
		);

  drupal_alter('cablecast_show_presave', $node, $d_project, $data);

  $show_id = cablecast_api_server_query('UpdateShowRecord', $data);

  return $show_id->UpdateShowRecordResult;
}

/**
 * Creates a Schedule record on Cablecast
 *
 * Updates an existing Schedule Record. All fields are required.
 * Blank or empty fields will overwrite existing show data.
 *
 * @param $data
 * @return $return
 */
function cablecast_api_add_airing($airing, $cablecast_show_id, $channel_name) {
  /*
    <s:element minOccurs="1" maxOccurs="1" name="ChannelID" type="s:int"/>
		<s:element minOccurs="1" maxOccurs="1" name="LocationID" type="s:int"/>
		<s:element minOccurs="1" maxOccurs="1" name="ShowID" type="s:int"/>
		<s:element minOccurs="1" maxOccurs="1" name="RunDateTime" type="s:dateTime"/>
		<s:element minOccurs="1" maxOccurs="1" name="RunLock" type="s:boolean"/>
		<s:element minOccurs="1" maxOccurs="1" name="RunBump" type="s:int"/>
		<s:element minOccurs="1" maxOccurs="1" name="CGExempt" type="s:boolean"/>
		<s:element minOccurs="0" maxOccurs="1" name="BugText" type="s:string"/>
		<s:element minOccurs="0" maxOccurs="1" name="CrawlText" type="s:string"/>
		<s:element minOccurs="1" maxOccurs="1" name="CrawlLength" type="s:int"/>
		<s:element minOccurs="0" maxOccurs="1" name="username" type="s:string"/>
		<s:element minOccurs="0" maxOccurs="1" name="password" type="s:string"/>
   */
  
  $start = 
    isset($airing->field_airing_date) &&
    isset($airing->field_airing_date[LANGUAGE_NONE]) &&
    isset($airing->field_airing_date[LANGUAGE_NONE][0]) &&
    isset($airing->field_airing_date[LANGUAGE_NONE][0]['value']) ?
    $airing->field_airing_date[LANGUAGE_NONE][0]['value'] : NULL;
  if (!$start) {
    drupal_set_message(t("Error Code 867619: ".
			 "We cannot do that for you right now."), 'error');
    return;
  }

  //BEGIN THE WORK TO PASS THE DATE TO CABLECAST IN CORRECT FORMAT
  $timestamp = strtotime($start);

  //FIXME: ADD AS A REAL SETTING
  $should_shift = variable_get('cablecast_api_apply_offset_on_airing_save', 1);

  $timestamp =  $should_shift ? 
    ($timestamp + date('Z', $timestamp)) : $timestamp;

  $rundatetime = date("c", $timestamp);

  // setup data array for SOAP call
  $data = array(
		'ChannelID'             => $channel_name,
		'LocationID'            => 1,
		'ShowID'           	=> $cablecast_show_id,
		'RunDateTime'           => $rundatetime,
		'RunLock'               => '',
		'RunBump'               => '',
		'CGExempt'              => FALSE, // TODO: may vary based on project type or other criteria
		'BugText'               => '',
		'CrawlText'             => '',
		'CrawlLength'           => '',
		);

  $airing_id = cablecast_api_server_query('AddScheduleEvent', $data);

  return $airing_id->AddScheduleEventResult;
}

/**
 * Returns a Cablecast ScheduleEvent for a Drupal Airing 
 *
 * @param $airing
 * @return $return
 */
function cablecast_api_get_airing($airing) {
  $schedule_id = 
    cablecast_api_get_single_field_value($airing, 
					 'field_cablecast_airing_id',
					 'value');
  if (!$schedule_id) {
    drupal_set_message("Could not find airing with id " . $airing->airing_id .
		       " because we could not find a Cablecast Schedule ID",
		       'warning');
    return;
  }

  $d_show_id = isset($airing->original) ? 
    cablecast_api_get_single_field_value($airing->original, 
					 'field_airing_show_ref',
					 'target_id') : NULL;

  $d_show_id = $d_show_id ? $d_show_id :
    cablecast_api_get_single_field_value($airing, 'field_airing_show_ref',
					 'target_id');

  $show_node = $d_show_id ? node_load($d_show_id) : NULL;

  $cc_show_id = $show_node ? 
    cablecast_api_get_single_field_value($show_node, 'field_cablecast_show_id',
					 'value') : 0;
  if (!$cc_show_id) {
    drupal_set_message("Cannot find airing for the show ". $show->title .
		       " as we cannot find a cablecast show id", 'error');
    return NULL;
  }

  $start = '2013-01-01';
  $end = date('Y-m-d', strtotime('now') + 5*60*60*24*365);

  $channel_vocab = taxonomy_vocabulary_machine_name_load('Channel');
  $terms = entity_load('taxonomy_term', FALSE, 
		       array('vid' => $channel_vocab->vid));

  reset($terms);
  foreach($terms as $tid=>$term) {
    $channel = cm_cablecast_convert_channel_tid_to_name($tid);

    // setup data array for SOAP call
    $data = array(
		  'ChannelID' => $channel,
		  'FromDate' => date('c', strtotime($start)),
		  'ToDate' => date('c', strtotime($end)),
		  'restrictToShowID' => $cc_show_id,
		  );

    $results = cablecast_api_server_query('GetScheduleInformation', $data);
    if ($results && 
	isset($results->GetScheduleInformationResult->ScheduleInfo)) {
      $schedule = $results->GetScheduleInformationResult->ScheduleInfo;
    }
    else {
      continue;
    }

    if (isset($schedule->ScheduleID)) {
      $schedule = array($schedule);
    }

    foreach($schedule as $cc_airing) {
      if ($schedule_id == $cc_airing->ScheduleID) {
	$cc_airing->drupal_channel_tid = $tid;
	return $cc_airing;
      }
    }
  }
  return NULL;
}
/**
 * @param $show
 *
 * Until the cablecast api supports show deletion, we will just empty
 * out the show and save it so it doesn't get in the way
 */
function cablecast_api_delete_show($show) {
 
  $cc_show_id = $show->field_cablecast_show_id[LANGUAGE_NONE][0]['value'];

  $cc_show = $cc_show_id ?
    cablecast_api_server_query('GetShowInformation',
			       array('ShowID'=>
				       $cc_show_id)) : NULL;
  $cc_show = ($cc_show && isset($cc_show->GetShowInformationResult)) ?
    $cc_show->GetShowInformationResult : NULL;

  // fetch the cablecast project id
  $d_project_id = 
    cablecast_api_get_single_field_value($show, 'og_group_ref',
					 'target_id');
  $d_project = $d_project_id ? node_load($d_project_id) : NULL;

  $cc_project_id = $d_project ?
    cablecast_api_get_single_field_value($d_project, 
					 'field_cablecast_project_id',
					 'value') : 0;

  // fetch the cablecast producer id
  $d_producer = $d_project ? user_load($d_project->uid) : 
    user_load($show->uid);
  $cc_producer_id = $d_producer ?
    cablecast_api_get_single_field_value($d_producer, 
					 'field_cablecast_producer_id',
					 'value') : 0;
  


  $custom_fields = array();

  // setup data array for SOAP call
  $data = array(
		'ShowID'                => $cc_show_id,
		'LocalID'               => "Added by API",
		'LocationID'            => 1,
		'Title'                 => 'DELETED SHOW',
		'CGTitle'               => 'DELETED SHOW',
		'ProjectID'             => $cc_project_id,
		// TODO: may vary based on project type or other criteria
		'CGExempt'              => FALSE, 
		'ProducerID'            => $cc_producer_id,
		'CategoryID'            => 1,
		'EventDate'             => date("c", $show->created),
		'Comments'              => $cc_show->Comments,
		'OnDemand'              => FALSE,
		'OnDemandStatus'        => FALSE,
		'BugText'               => '',
		'CrawlText'             => '',
		'CrawlLengthInSeconds'  => '',
		'TotalSeconds'           => 1,
		'CustomFields'          => $custom_fields,
		
		);

  drupal_alter('cablecast_show_delete', $show, $d_project, $data);
  $show_id = cablecast_api_server_query('UpdateShowRecord', $data);
  return $show_id->UpdateShowRecordResult;

}
/**

 *
 * @param $airing
 * @return $return
 */
function cablecast_api_delete_airing($airing) {

  $schedule_id =
    (isset($airing->field_cablecast_airing_id) &&
     isset($airing->field_cablecast_airing_id[LANGUAGE_NONE]) &&
     isset($airing->field_cablecast_airing_id[LANGUAGE_NONE][0]) &&
     isset($airing->field_cablecast_airing_id[LANGUAGE_NONE][0]['value'])) ?
    $airing->field_cablecast_airing_id[LANGUAGE_NONE][0]['value'] : NULL;

  $channel_tid =
    (isset($airing->field_airing_channel) &&
     isset($airing->field_airing_channel[LANGUAGE_NONE]) &&
     isset($airing->field_airing_channel[LANGUAGE_NONE][0]) &&
     isset($airing->field_airing_channel[LANGUAGE_NONE][0]['tid'])) ?
    $airing->field_airing_channel[LANGUAGE_NONE][0]['tid'] : NULL;

  $channel = cm_cablecast_convert_channel_tid_to_name($channel_tid);
  if (!$schedule_id) {
    drupal_set_message("Did not delete airing with id " . $airing->airing_id .
		       " because we could not find a Cablecast Schedule ID",
		       'warning');
    return;
  }
  // setup data array for SOAP call
  $data = array(
		'ScheduleID' => $schedule_id,
		'ChannelID' => $channel,
		'LocationID'    => 1,
		);

  $return = cablecast_api_server_query('RemoveScheduleEvent', $data);
  

  return $return->RemoveScheduleEventResult;
}

/**
 * Updates a Show on Cablecast
 *
 * @param $data
 * @return $return
 */
function cablecast_api_get_show($node) {
  /*
    <ShowID>int</ShowID>
   */
  
  // setup data array for SOAP call
  $data = array(
    'ShowID'                => $node->field_cablecast_show_id[LANGUAGE_NONE][0]['value'],
  );
  
  $return = cablecast_api_server_query('GetShowInformation', $data);

  return $return->GetShowInformationResult->ShowID;
}

/**
 * Returns shows at this site that were modified in the range requested by the
 * SearchDate and DateComarator. DateComparator is a string consisting of
 * "<", ">", "<=", or ">="
 */
function cablecast_api_last_modified_search($data) {
  /*
         <LocationID>int</LocationID>
         <SearchDate>dateTime</SearchDate>
         <DateComparator>string</DateComparator>
   */

  $return = cablecast_api_server_query('LastModifiedSearch', $data);
  return $return->LastModifiedSearchResult->ShowInfo;
}

/**
 * Returns a list of all the channels on this Cablecast server.
 *
 * @return mixed
 */
function cablecast_api_get_channels() {
  /*
     <GetChannels xmlns="http://www.trms.com/CablecastWS/" />
   */

  $return = cablecast_api_server_query('GetChannels', $params = array());

  return $return->GetChannelsResult->Channel;

}

function cablecast_api_get_schedule($data) {
  /*
       <GetScheduleInformation xmlns="http://www.trms.com/CablecastWS/">
         <ChannelID>int</ChannelID>
         <FromDate>dateTime</FromDate>
         <ToDate>dateTime</ToDate>
         <restrictToShowID>int</restrictToShowID>
       </GetScheduleInformation>
   */

  $return = cablecast_api_server_query('GetScheduleInformation', $data);
  return $return->GetScheduleInformationResult->ScheduleInfo;

}

function cablecast_api_get_version() {
  if (!variable_get('cablecast_api_server_address', NULL)) {
    drupal_set_message(t('<a href="@url">Cablecast API must be configured.</a>', array('@url' => url('admin/config/cablecast-api'))), 'error');
    return;
  }
  return cablecast_api_server_query('DataVersion', $data = array())->DataVersionResult;
}

function cablecast_api_get_locations() {
  return cablecast_api_server_query('GetLocations', $data = array());
}


/**
 * Helper function to get a single value off of a entity
 */
function cablecast_api_get_single_field_value($entity, $field_name, 
					  $index = 'value') {
  if (isset($entity->{$field_name})) {
    $field = $entity->{$field_name};
    if (isset($field[LANGUAGE_NONE]) && 
	isset($field[LANGUAGE_NONE][0]) &&
	isset($field[LANGUAGE_NONE][0][$index])) {
      return $field[LANGUAGE_NONE][0][$index];
    }
  }
  return NULL;
}
