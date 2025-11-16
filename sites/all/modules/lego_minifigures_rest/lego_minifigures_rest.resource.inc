<?php

/**
 * @file
 * Callback implementations for the Lego minifigure REST resource.
 * Uses Drupal Node API instead of variable storage.
 */

/**
 * Defines the lego_minifigures REST resource.
 *
 * @return array
 *   The resource definition compatible with Services.
 */
function _lego_minifigures_rest_definition() {
  $file = array(
    'type' => 'inc',
    'module' => 'lego_minifigures_rest',
    'name' => 'lego_minifigures_rest.resource',
  );

  return array(
    'lego_minifigures' => array(
      'operations' => array(
        'index' => array(
          'help' => 'List all Lego minifigures.',
          'callback' => '_lego_minifigures_rest_index',
          'file' => $file,
          'access callback' => '_lego_minifigures_rest_access',
          'access arguments' => array('index'),
          'access arguments append' => TRUE,
          'args' => array(
            array(
              'name' => 'offset',
              'type' => 'int',
              'description' => 'Zero-based offset of the first record to return.',
              'optional' => TRUE,
              'default value' => 0,
              'source' => array('param' => 'offset'),
            ),
            array(
              'name' => 'limit',
              'type' => 'int',
              'description' => 'Maximum number of records to return.',
              'optional' => TRUE,
              'default value' => 25,
              'source' => array('param' => 'limit'),
            ),
          ),
        ),
        'retrieve' => array(
          'help' => 'Retrieve a single Lego minifigure.',
          'callback' => '_lego_minifigures_rest_retrieve',
          'file' => $file,
          'access callback' => '_lego_minifigures_rest_access',
          'access arguments' => array('retrieve'),
          'access arguments append' => TRUE,
          'args' => array(
            array(
              'name' => 'id',
              'type' => 'int',
              'description' => 'Identifier (nid) of the minifigure to retrieve.',
              'optional' => FALSE,
              'source' => array('path' => 0),
            ),
          ),
        ),
        'create' => array(
          'help' => 'Create a new Lego minifigure.',
          'callback' => '_lego_minifigures_rest_create',
          'file' => $file,
          'access callback' => '_lego_minifigures_rest_access',
          'access arguments' => array('create'),
          'access arguments append' => TRUE,
          'args' => array(
            array(
              'name' => 'data',
              'type' => 'array',
              'description' => 'Payload describing the minifigure to create.',
              'optional' => FALSE,
              'source' => 'data',
            ),
          ),
        ),
        'update' => array(
          'help' => 'Update an existing Lego minifigure.',
          'callback' => '_lego_minifigures_rest_update',
          'file' => $file,
          'access callback' => '_lego_minifigures_rest_access',
          'access arguments' => array('update'),
          'access arguments append' => TRUE,
          'args' => array(
            array(
              'name' => 'id',
              'type' => 'int',
              'description' => 'Identifier (nid) of the minifigure to update.',
              'optional' => FALSE,
              'source' => array('path' => 0),
            ),
            array(
              'name' => 'data',
              'type' => 'array',
              'description' => 'Payload containing updates for the minifigure.',
              'optional' => FALSE,
              'source' => 'data',
            ),
          ),
        ),
        'delete' => array(
          'help' => 'Delete a Lego minifigure.',
          'callback' => '_lego_minifigures_rest_delete',
          'file' => $file,
          'access callback' => '_lego_minifigures_rest_access',
          'access arguments' => array('delete'),
          'access arguments append' => TRUE,
          'args' => array(
            array(
              'name' => 'id',
              'type' => 'int',
              'description' => 'Identifier (nid) of the minifigure to delete.',
              'optional' => FALSE,
              'source' => array('path' => 0),
            ),
          ),
        ),
      ),
    ),
  );
}

/**
 * Returns a paginated list of minifigure nodes.
 *
 * @param int $offset
 *   Optional offset from the beginning of the result set.
 * @param int $limit
 *   Optional maximum number of records to return.
 *
 * @return array
 *   A structured response containing the requested data.
 */
function _lego_minifigures_rest_index($offset = 0, $limit = 25) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'minifigure')
    ->propertyCondition('status', NODE_PUBLISHED)
    ->propertyOrderBy('title', 'ASC')
    ->range($offset, $limit);

  $result = $query->execute();

  if (empty($result['node'])) {
    return array(
      'data' => array(),
      'count' => 0,
      'offset' => $offset,
      'limit' => $limit,
    );
  }

  $nids = array_keys($result['node']);
  $nodes = node_load_multiple($nids);
  $items = array();

  foreach ($nodes as $node) {
    $items[] = _lego_minifigures_rest_node_to_array($node);
  }

  // Get total count for pagination.
  $count_query = new EntityFieldQuery();
  $count_query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'minifigure')
    ->propertyCondition('status', NODE_PUBLISHED);
  $count_result = $count_query->execute();
  $total_count = isset($count_result['node']) ? count($count_result['node']) : 0;

  return array(
    'data' => $items,
    'count' => $total_count,
    'offset' => $offset,
    'limit' => $limit,
  );
}

/**
 * Retrieves a single minifigure node by its nid.
 *
 * @param int $id
 *   The node ID (nid) of the requested item.
 *
 * @return array
 *   The minifigure data as an array.
 *
 * @throws \Exception
 *   Thrown when the requested item cannot be found.
 */
function _lego_minifigures_rest_retrieve($id) {
  $node = node_load($id);

  if (!$node || $node->type !== 'minifigure') {
    _lego_minifigures_rest_throw(t('Minifigure @id was not found.', array('@id' => $id)), 404);
  }

  return _lego_minifigures_rest_node_to_array($node);
}

/**
 * Creates a new minifigure node based on POSTed data.
 *
 * @param array $data
 *   The decoded payload sent by the client.
 *
 * @return array
 *   The newly created minifigure data.
 *
 * @throws \Exception
 *   Thrown when validation fails.
 */
function _lego_minifigures_rest_create($data = array()) {
  $payload = _lego_minifigures_rest_extract_payload($data);

  // Create new node object.
  $node = new stdClass();
  $node->type = 'minifigure';
  $node->language = LANGUAGE_NONE;
  $node->uid = isset($GLOBALS['user']->uid) ? $GLOBALS['user']->uid : 0;
  $node->status = NODE_PUBLISHED;
  $node->promote = 0;
  $node->comment = 0;

  // Set title (required).
  if (empty($payload['title'])) {
    _lego_minifigures_rest_throw(t('The "title" field is required.'), 400);
  }
  $node->title = trim($payload['title']);

  // Set field_name.
  if (isset($payload['field_name'])) {
    $node->field_name[LANGUAGE_NONE][0]['value'] = trim($payload['field_name']);
  }

  // Set field_description.
  if (isset($payload['field_description'])) {
    $node->field_description[LANGUAGE_NONE][0]['value'] = trim($payload['field_description']);
  }

  // Set field_image (expects file ID or file object).
  if (isset($payload['field_image'])) {
    $node->field_image[LANGUAGE_NONE][0] = _lego_minifigures_rest_prepare_image_field($payload['field_image']);
  }

  // Validate and save the node.
  node_object_prepare($node);
  $node = node_submit($node);

  if ($errors = node_validate($node)) {
    $error_messages = array();
    foreach ($errors as $field => $field_errors) {
      foreach ($field_errors as $error) {
        $error_messages[] = $error['message'];
      }
    }
    _lego_minifigures_rest_throw(implode(' ', $error_messages), 400);
  }

  node_save($node);

  return _lego_minifigures_rest_node_to_array($node);
}

/**
 * Updates an existing minifigure node.
 *
 * @param int $id
 *   The node ID (nid) of the minifigure to update.
 * @param array $data
 *   The decoded payload sent by the client.
 *
 * @return array
 *   The updated minifigure data.
 *
 * @throws \Exception
 *   Thrown when validation fails or the minifigure cannot be found.
 */
function _lego_minifigures_rest_update($id, $data = array()) {
  $node = node_load($id);

  if (!$node || $node->type !== 'minifigure') {
    _lego_minifigures_rest_throw(t('Minifigure @id was not found.', array('@id' => $id)), 404);
  }

  $payload = _lego_minifigures_rest_extract_payload($data);

  // Update title if provided.
  if (isset($payload['title'])) {
    $node->title = trim($payload['title']);
  }

  // Update field_name if provided.
  if (isset($payload['field_name'])) {
    $node->field_name[LANGUAGE_NONE][0]['value'] = trim($payload['field_name']);
  }

  // Update field_description if provided.
  if (isset($payload['field_description'])) {
    $node->field_description[LANGUAGE_NONE][0]['value'] = trim($payload['field_description']);
  }

  // Update field_image if provided.
  if (isset($payload['field_image'])) {
    $node->field_image[LANGUAGE_NONE][0] = _lego_minifigures_rest_prepare_image_field($payload['field_image']);
  }

  // Validate and save the node.
  node_object_prepare($node);
  $node = node_submit($node);

  if ($errors = node_validate($node)) {
    $error_messages = array();
    foreach ($errors as $field => $field_errors) {
      foreach ($field_errors as $error) {
        $error_messages[] = $error['message'];
      }
    }
    _lego_minifigures_rest_throw(implode(' ', $error_messages), 400);
  }

  node_save($node);

  return _lego_minifigures_rest_node_to_array($node);
}

/**
 * Deletes a minifigure node by its nid.
 *
 * @param int $id
 *   The node ID (nid) of the minifigure to delete.
 *
 * @return array
 *   A confirmation payload.
 *
 * @throws \Exception
 *   Thrown when the minifigure cannot be found.
 */
function _lego_minifigures_rest_delete($id) {
  $node = node_load($id);

  if (!$node || $node->type !== 'minifigure') {
    _lego_minifigures_rest_throw(t('Minifigure @id was not found.', array('@id' => $id)), 404);
  }

  $title = $node->title;
  node_delete($id);

  return array(
    'status' => 'deleted',
    'id' => $id,
    'title' => $title,
  );
}

/**
 * Converts a node object to an array for API response.
 *
 * @param object $node
 *   The node object.
 *
 * @return array
 *   The minifigure data as an array.
 */
function _lego_minifigures_rest_node_to_array($node) {
  $data = array(
    'id' => $node->nid,
    'title' => $node->title,
    'created' => $node->created,
    'updated' => $node->changed,
  );

  // Extract field_name.
  if (isset($node->field_name[LANGUAGE_NONE][0]['value'])) {
    $data['field_name'] = $node->field_name[LANGUAGE_NONE][0]['value'];
  }

  // Extract field_description.
  if (isset($node->field_description[LANGUAGE_NONE][0]['value'])) {
    $data['field_description'] = $node->field_description[LANGUAGE_NONE][0]['value'];
  }

  // Extract field_image.
  if (isset($node->field_image[LANGUAGE_NONE][0])) {
    $file = $node->field_image[LANGUAGE_NONE][0];
    $data['field_image'] = array(
      'fid' => $file['fid'],
      'filename' => $file['filename'],
      'uri' => $file['uri'],
      'url' => file_create_url($file['uri']),
    );
  }

  return $data;
}

/**
 * Prepares image field data for node save.
 *
 * @param mixed $image_data
 *   Can be a file ID (int), file object, or array with file info.
 *
 * @return array
 *   Formatted image field array.
 */
function _lego_minifigures_rest_prepare_image_field($image_data) {
  // If it's already a file array, return it.
  if (is_array($image_data) && isset($image_data['fid'])) {
    return $image_data;
  }

  // If it's a file ID, load the file.
  if (is_numeric($image_data)) {
    $file = file_load($image_data);
    if (!$file) {
      _lego_minifigures_rest_throw(t('File with ID @fid was not found.', array('@fid' => $image_data)), 400);
    }
    return array(
      'fid' => $file->fid,
      'uid' => $file->uid,
      'filename' => $file->filename,
      'uri' => $file->uri,
      'filemime' => $file->filemime,
      'filesize' => $file->filesize,
      'status' => $file->status,
      'timestamp' => $file->timestamp,
    );
  }

  // If it's a file object.
  if (is_object($image_data) && isset($image_data->fid)) {
    return array(
      'fid' => $image_data->fid,
      'uid' => $image_data->uid,
      'filename' => $image_data->filename,
      'uri' => $image_data->uri,
      'filemime' => $image_data->filemime,
      'filesize' => $image_data->filesize,
      'status' => $image_data->status,
      'timestamp' => $image_data->timestamp,
    );
  }

  _lego_minifigures_rest_throw(t('Invalid image field data provided.'), 400);
}

/**
 * Extracts payload data from Services or raw request content.
 *
 * @param array $data
 *   The payload provided by Services.
 *
 * @return array
 *   The sanitized payload.
 */
function _lego_minifigures_rest_extract_payload($data = array()) {
  if (!empty($data) && is_array($data)) {
    return $data;
  }

  return _lego_minifigures_rest_read_request_body();
}

/**
 * Reads a JSON payload from the request body.
 *
 * @return array
 *   The decoded payload.
 *
 * @throws \Exception
 *   Thrown when the payload cannot be decoded.
 */
function _lego_minifigures_rest_read_request_body() {
  $raw = file_get_contents('php://input');
  if ($raw === FALSE || $raw === '') {
    _lego_minifigures_rest_throw(t('Empty request body.'), 400);
  }

  $decoded = drupal_json_decode($raw);
  if (!is_array($decoded)) {
    _lego_minifigures_rest_throw(t('Unable to decode the JSON payload.'), 400);
  }

  return $decoded;
}

/**
 * Determines whether the current user can perform an operation.
 *
 * @param string $op
 *   The requested operation.
 * @param array $args
 *   Arguments passed alongside the operation.
 * @param object|null $account
 *   Optional user account object.
 *
 * @return bool
 *   TRUE if access should be granted, FALSE otherwise.
 */
function _lego_minifigures_rest_access($op, array $args = array(), $account = NULL) {
  if (!$account) {
    global $user;
    $account = $user;
  }

  switch ($op) {
    case 'create':
      return user_access('create minifigure content', $account);

    case 'update':
      if (!empty($args[0])) {
        $node = node_load($args[0]);
        if ($node && $node->type === 'minifigure') {
          return node_access('update', $node, $account);
        }
      }
      return user_access('edit any minifigure content', $account);

    case 'delete':
      if (!empty($args[0])) {
        $node = node_load($args[0]);
        if ($node && $node->type === 'minifigure') {
          return node_access('delete', $node, $account);
        }
      }
      return user_access('delete any minifigure content', $account);

    case 'index':
    case 'retrieve':
    default:
      return user_access('access content', $account);
  }
}

/**
 * Throws a Services-friendly exception.
 *
 * @param string $message
 *   The error message.
 * @param int $code
 *   The HTTP status code.
 *
 * @throws \Exception
 */
function _lego_minifigures_rest_throw($message, $code = 400) {
  if (class_exists('ServicesException')) {
    throw new ServicesException($message, $code);
  }

  throw new Exception($message, $code);
}
