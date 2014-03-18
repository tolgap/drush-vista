<?php

/**
 * Missing command.
 */
function pm_release_recommended(&$project) {
  if (isset($project['recommended'])) {
    $project['candidate_version'] = $project['recommended'];
    $project['updateable'] = TRUE;
  }
  // If installed version is dev and the candidate version is older, choose
  // latest dev as candidate.
  if (($project['install_type'] == 'dev') && isset($project['candidate_version'])) {
    if ($project['releases'][$project['candidate_version']]['date'] < $project['info']['datestamp']) {
      $project['candidate_version'] = $project['latest_dev'];
      if ($project['releases'][$project['candidate_version']]['date'] <= $project['info']['datestamp']) {
        $project['candidate_version'] = $project['existing_version'];
        $project['updateable'] = FALSE;
      }
    }
  }
}

function vista_drush_command() {
  $items = array();

  $items['vista-modules'] = array(
    'description' => "Print a JSON array with module (update) information.",
    'aliases' => array('vista-m'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'drupal dependencies' => array(
      'update',
    ),
    'drush dependencies' => array(
      'pm',
    ),
  );

  $items['vista-name'] = array(
    'description' => "Return the site name.",
    'aliases' => array('vista-n'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'drupal dependencies' => array(),
    'drush dependencies' => array(),
  );

  $items['vista-core-version'] = array(
    'description' => "Return the site version.",
    'aliases' => array('vista-cv'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'drupal dependencies' => array(),
    'drush dependencies' => array(),
  );

  return $items;
}

function vista_drush_help($section) {
  switch($section) {
    case 'drush:vista-modules':
      return dt('Outputs all module information in JSON format.');
    case 'drush:vista-name':
      return dt('Outputs the site name.');
    case 'drush:vista-core-version':
      return dt('Outputs Drupal core version.');
    case 'meta:vista:title':
      return dt('Vista commands');
    case 'meta:vista:summary':
      return dt('Vista provides JSON formatting to module and core information.');
  }
}

function drush_vista_core_version($name='system') {
  $path = drupal_get_path('module', $name) . '/' . $name . '.info';
  $info = drupal_parse_info_file($path);
  echo $info["version"] . PHP_EOL;
}

function drush_vista_name() {
  $site_name = variable_get('site_name', "");
  echo $site_name . PHP_EOL;
}

function drush_vista_modules() {
  // We don't provide for other options here, so we supply an explicit path.
  drush_include_engine('update_info', 'drupal', NULL, DRUSH_BASE_PATH . '/commands/pm/update_info');

  // Get installed extensions and projects.
  $extensions = drush_get_extensions();
  $projects = drush_get_projects($extensions);

  $update_info = _pm_get_update_info($projects);

  foreach ($extensions as $name => $extension) {
    // Add an item to $update_info for each enabled extension which was obtained
    // from cvs or git and its project is unknown (because of cvs_deploy or
    // git_deploy is not enabled).
    if (!isset($extension->info['project'])) {
      if ((isset($extension->vcs)) && ($extension->status)) {
        $update_info[$name] = array(
          'name' => $name,
          'label' => $extension->label,
          'existing_version' => 'Unknown',
          'status' => 'VCS',
          'status_msg' => dt('Project was not packaged by drupal.org but obtained from !vcs. You need to enable !vcs_deploy module', array('!vcs' => $extension->vcs)),
        );
      }
    }
  }

  // Process locks specified on the command line.
  $locked_list = drush_pm_update_lock($update_info, drush_get_option_list('lock'), drush_get_option_list('unlock'), drush_get_option('lock-message'));

  // Build project updatable messages, set candidate version and mark
  // 'updatable' in the project.
  foreach ($update_info as $key => $project) {
    $status = pm_update_filter($project);

    if (isset($project['locked'])) {
      $status = $project['locked'] . " ($status)";
    }
    // Persist candidate_version in $update_info (plural).
    if (empty($project['candidate_version'])) {
      $update_info[$key]['candidate_version'] = $project['existing_version']; // Default to no change
    }
    else {
      $update_info[$key]['candidate_version'] = $project['candidate_version'];
    }
    $update_info[$key]['status_msg'] = $status;
    if (isset($project['updateable'])) {
      $update_info[$key]['updateable'] = $project['updateable'];
    }
  }

  $data = _vista_sanitize_output($update_info);

  echo json_encode($data) . PHP_EOL;
}

function _vista_sanitize_output($update_info) {
  $sanitized = array();
  foreach ($update_info as $key => $project) {
    // Reformat the status code
    switch($project["status"]) {
      case UPDATE_CURRENT:
        $update = "none";
        break;
      case UPDATE_NOT_CURRENT:
        $update = "available";
        break;
      case UPDATE_NOT_SECURE:
        $update = "security";
        break;
      default:
        $update = "failed";
        break;
    }

    // Humanized project status label
    $status = ($project["project_status"] ? "active" : "inactive");

    $sanitized[] = array(
      "name" => $project["name"],
      "version" => $project["info"]["version"],
      "status" => $status,
      "update" => $update
    );
  }

  return $sanitized;
}