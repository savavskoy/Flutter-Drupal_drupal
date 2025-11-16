<?php

/**
 * @file
 * Callback implementations for the Lego minifigure REST resource.
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
              'description' => 'Identifier of the minifigure to retrieve.',
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
              'description' => 'Identifier of the minifigure to update.',
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
              'description' => 'Identifier of the minifigure to delete.',
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
 * Returns a paginated list of stored minifigures.
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
  $items = _lego_minifigures_rest_load_items();
  uasort($items, '_lego_minifigures_rest_sort_items');

  $offset = max(0, (int) $offset);
  $limit = (int) $limit;
  if ($limit <= 0) {
    $limit = count($items);
  }

  $paged_items = array_slice(array_values($items), $offset, $limit);

  return array(
    'data' => $paged_items,
    'count' => count($items),
    'offset' => $offset,
    'limit' => $limit,
  );
}

/**
 * Retrieves a single minifigure by its identifier.
 *
 * @param int $id
 *   The numeric identifier of the requested item.
 *
 * @return array
 *   The stored item.
 *
 * @throws \Exception
 *   Thrown when the requested item cannot be found.
 */
function _lego_minifigures_rest_retrieve($id) {
  $items = _lego_minifigures_rest_load_items();
  $id = (int) $id;

  if (!isset($items[$id])) {
    _lego_minifigures_rest_throw(t('Minifigure @id was not found.', array('@id' => $id)), 404);
  }

  return $items[$id];
}

/**
 * Persists a new minifigure based on POSTed data.
 *
 * @param array $data
 *   The decoded payload sent by the client.
 *
 * @return array
 *   The newly created item.
 *
 * @throws \Exception
 *   Thrown when validation fails.
 */
function _lego_minifigures_rest_create($data = array()) {
  $payload = _lego_minifigures_rest_extract_payload($data);
  $items = _lego_minifigures_rest_load_items();

  $item = _lego_minifigures_rest_prepare_item($payload);
  $id = _lego_minifigures_rest_generate_id($items);
  $timestamp = REQUEST_TIME;

  $item['id'] = $id;
  $item['created'] = $timestamp;
  $item['updated'] = $timestamp;

  $items[$id] = $item;
  _lego_minifigures_rest_save_items($items);

  return $items[$id];
}

/**
 * Updates an existing minifigure.
 *
 * @param int $id
 *   The identifier of the minifigure to update.
 * @param array $data
 *   The decoded payload sent by the client.
 *
 * @return array
 *   The updated minifigure.
 *
 * @throws \Exception
 *   Thrown when validation fails or the minifigure cannot be found.
 */
function _lego_minifigures_rest_update($id, $data = array()) {
  $items = _lego_minifigures_rest_load_items();
  $id = (int) $id;

  if (!isset($items[$id])) {
    _lego_minifigures_rest_throw(t('Minifigure @id was not found.', array('@id' => $id)), 404);
  }

  $payload = _lego_minifigures_rest_extract_payload($data);
  $updated_item = _lego_minifigures_rest_prepare_item($payload, $items[$id], TRUE);
  $updated_item['id'] = $id;
  $updated_item['created'] = $items[$id]['created'];
  $updated_item['updated'] = REQUEST_TIME;

  $items[$id] = $updated_item;
  _lego_minifigures_rest_save_items($items);

  return $items[$id];
}

/**
 * Deletes a minifigure by its identifier.
 *
 * @param int $id
 *   The identifier of the minifigure to delete.
 *
 * @return array
 *   A confirmation payload.
 *
 * @throws \Exception
 *   Thrown when the minifigure cannot be found.
 */
function _lego_minifigures_rest_delete($id) {
  $items = _lego_minifigures_rest_load_items();
  $id = (int) $id;

  if (!isset($items[$id])) {
    _lego_minifigures_rest_throw(t('Minifigure @id was not found.', array('@id' => $id)), 404);
  }

  $deleted = $items[$id];
  unset($items[$id]);
  _lego_minifigures_rest_save_items($items);

  return array(
    'status' => 'deleted',
    'id' => $deleted['id'],
    'name' => $deleted['name'],
  );
}

/**
 * Loads all stored items from Drupal variable storage.
 *
 * @return array
 *   The stored items indexed by their ID.
 */
function _lego_minifigures_rest_load_items() {
  $items = variable_get('lego_minifigures_rest_items', array());
  return is_array($items) ? $items : array();
}

/**
 * Saves the full collection of items to Drupal variable storage.
 *
 * @param array $items
 *   The items to persist.
 */
function _lego_minifigures_rest_save_items(array $items) {
  variable_set('lego_minifigures_rest_items', $items);
}

/**
 * Generates a new integer identifier based on existing items.
 *
 * @param array $items
 *   The existing items.
 *
 * @return int
 *   A unique identifier.
 */
function _lego_minifigures_rest_generate_id(array $items) {
  if (empty($items)) {
    return 1;
  }

  return max(array_keys($items)) + 1;
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
 * Normalizes and validates incoming data for create/update operations.
 *
 * @param array $payload
 *   The raw payload from the client.
 * @param array $existing
 *   The existing minifigure (only for updates).
 * @param bool $is_update
 *   Whether the normalization is performed for an update.
 *
 * @return array
 *   The normalized minifigure array.
 */
function _lego_minifigures_rest_prepare_item(array $payload, array $existing = array(), $is_update = FALSE) {
  $base = array(
    'name' => '',
    'theme' => '',
    'release_year' => NULL,
    'condition' => '',
    'notes' => '',
    'accessories' => array(),
    'owned' => TRUE,
  );

  $item = $existing + $base;

  // Name.
  if (array_key_exists('name', $payload)) {
    $name = trim($payload['name']);
    if ($name === '') {
      _lego_minifigures_rest_throw(t('The "name" field is required.'), 400);
    }
    if (drupal_strlen($name) > 255) {
      _lego_minifigures_rest_throw(t('The "name" field must not exceed @length characters.', array('@length' => 255)), 400);
    }
    $item['name'] = $name;
  }
  elseif (!$is_update || $item['name'] === '') {
    _lego_minifigures_rest_throw(t('The "name" field is required.'), 400);
  }

  // Theme.
  if (array_key_exists('theme', $payload)) {
    $theme = trim($payload['theme']);
    if (drupal_strlen($theme) > 255) {
      _lego_minifigures_rest_throw(t('The "theme" field must not exceed @length characters.', array('@length' => 255)), 400);
    }
    $item['theme'] = $theme;
  }

  // Release year.
  if (array_key_exists('release_year', $payload)) {
    if ($payload['release_year'] === '' || $payload['release_year'] === NULL) {
      $item['release_year'] = NULL;
    }
    elseif (!is_numeric($payload['release_year'])) {
      _lego_minifigures_rest_throw(t('The "release_year" field must be a number.'), 400);
    }
    else {
      $year = (int) $payload['release_year'];
      if ($year < 1970 || $year > (int) format_date(REQUEST_TIME, 'custom', 'Y') + 2) {
        _lego_minifigures_rest_throw(t('The "release_year" field must be between 1970 and the near future.'), 400);
      }
      $item['release_year'] = $year;
    }
  }

  // Condition.
  if (array_key_exists('condition', $payload)) {
    $condition = trim($payload['condition']);
    if (drupal_strlen($condition) > 255) {
      _lego_minifigures_rest_throw(t('The "condition" field must not exceed @length characters.', array('@length' => 255)), 400);
    }
    $item['condition'] = $condition;
  }

  // Notes.
  if (array_key_exists('notes', $payload)) {
    $notes = trim($payload['notes']);
    $item['notes'] = $notes;
  }

  // Accessories.
  if (array_key_exists('accessories', $payload)) {
    $item['accessories'] = _lego_minifigures_rest_normalize_accessories($payload['accessories']);
  }

  // Owned flag.
  if (array_key_exists('owned', $payload)) {
    $owned = _lego_minifigures_rest_normalize_boolean($payload['owned']);
    if ($owned === NULL) {
      _lego_minifigures_rest_throw(t('The "owned" field must be boolean.'), 400);
    }
    $item['owned'] = $owned;
  }

  return $item;
}

/**
 * Normalizes accessories to a string array.
 *
 * @param mixed $accessories
 *   The incoming accessories value.
 *
 * @return array
 *   A list of accessory strings.
 */
function _lego_minifigures_rest_normalize_accessories($accessories) {
  if ($accessories === NULL || $accessories === '') {
    return array();
  }

  if (!is_array($accessories)) {
    _lego_minifigures_rest_throw(t('The "accessories" field must be an array.'), 400);
  }

  $normalized = array();
  foreach ($accessories as $accessory) {
    $value = trim((string) $accessory);
    if ($value !== '') {
      $normalized[] = $value;
    }
  }

  return array_values($normalized);
}

/**
 * Converts a value to boolean if possible.
 *
 * @param mixed $value
 *   The value to normalize.
 *
 * @return bool|null
 *   TRUE or FALSE on success, NULL on failure.
 */
function _lego_minifigures_rest_normalize_boolean($value) {
  if (is_bool($value)) {
    return $value;
  }

  if (is_string($value)) {
    $value = drupal_strtolower($value);
    if (in_array($value, array('1', 'true', 'yes'), TRUE)) {
      return TRUE;
    }
    if (in_array($value, array('0', 'false', 'no'), TRUE)) {
      return FALSE;
    }
  }

  if (is_numeric($value)) {
    return ((int) $value) === 1;
  }

  return NULL;
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
 * Sort callback to order minifigures by name.
 *
 * @param array $a
 *   First item.
 * @param array $b
 *   Second item.
 *
 * @return int
 *   Comparison result.
 */
function _lego_minifigures_rest_sort_items(array $a, array $b) {
  return strcasecmp($a['name'], $b['name']);
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
    case 'update':
    case 'delete':
      return user_access('access content', $account);

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

