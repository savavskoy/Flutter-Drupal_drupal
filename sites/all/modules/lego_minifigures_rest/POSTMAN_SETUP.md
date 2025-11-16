# Postman Collection Setup Guide

## Quick Start

1. **Import the Collection**
   - Open Postman
   - Click "Import" button
   - Select `LEGO_Minifigures_API.postman_collection.json`
   - The collection will appear in your Postman sidebar

2. **Configure Variables**
   - Click on the collection name "Lego Minifigures REST API"
   - Go to the "Variables" tab
   - Update `base_url` to match your Drupal installation:
     - MAMP default: `http://localhost:8888/drupal`
     - XAMPP default: `http://localhost/drupal`
     - Custom: `http://your-domain.com`
   - Update `username` and `password` with your Drupal credentials
   - The `minifigure_id` variable will be updated automatically when you create a minifigure

3. **Login First (Recommended)**
   - Go to "Authentication" folder → "POST - User Login"
   - Update `username` and `password` variables if needed
   - Send the request
   - Session token and user info will be automatically saved
   - Now you can use authenticated endpoints

4. **Test the API**
   - Start with "GET - List All Minifigures" (should return empty array if no minifigures exist)
   - Create a minifigure using "POST - Create Minifigure (Minimal)"
   - Copy the `id` from the response and update the `minifigure_id` variable
   - Test other operations (GET single, UPDATE, DELETE)

## Authentication Endpoints

### Login
- **POST** `{{base_url}}/api/v1/user/login`
- Body: `{"username": "your_username", "password": "your_password"}`
- Returns: Session token (`sessid`), session name, and user object
- **Automatically saves session cookies and token** for subsequent requests

### Logout
- **POST** `{{base_url}}/api/v1/user/logout`
- Requires: Authentication (session cookies + CSRF token)
- Clears the current session

### Register
- **POST** `{{base_url}}/api/v1/user/register`
- Body: `{"name": "username", "mail": "email@example.com", "pass": "password"}`
- Creates a new user account
- Returns: Created user object

### Get CSRF Token
- **GET** `{{base_url}}/api/v1/user/token`
- Returns: CSRF token string for authenticated requests

### Get Current User
- **GET** `{{base_url}}/api/v1/user/{{user_id}}`
- Requires: Authentication
- Returns: Current logged-in user details

## API Endpoints

### Base URL Structure
```
{{base_url}}/api/v1/lego_minifigures
```

**Note:** The `/api/v1` path depends on your Services endpoint configuration. If you configured a different endpoint path, update it in the collection.

### Endpoints

1. **GET - List All Minifigures**
   - URL: `{{base_url}}/api/v1/lego_minifigures?offset=0&limit=25`
   - Query params: `offset` (optional), `limit` (optional)
   - Returns: Array of minifigures with pagination info

2. **GET - Get Single Minifigure**
   - URL: `{{base_url}}/api/v1/lego_minifigures/{{minifigure_id}}`
   - Returns: Single minifigure object

3. **POST - Create Minifigure**
   - URL: `{{base_url}}/api/v1/lego_minifigures`
   - Body: JSON with `title` (required), `field_name`, `field_description`, `field_image`
   - Returns: Created minifigure object with `id` (nid)

4. **PUT - Update Minifigure**
   - URL: `{{base_url}}/api/v1/lego_minifigures/{{minifigure_id}}`
   - Body: JSON with fields to update (all optional)
   - Returns: Updated minifigure object

5. **DELETE - Delete Minifigure**
   - URL: `{{base_url}}/api/v1/lego_minifigures/{{minifigure_id}}`
   - Returns: Confirmation object with status, id, and title

## Request Body Examples

### Create Minifigure (Full)
```json
{
  "title": "Classic Space Astronaut",
  "field_name": "Space Explorer",
  "field_description": "A classic blue space astronaut minifigure from the 1980s",
  "field_image": {
    "fid": 1
  }
}
```

### Create Minifigure (Minimal - Required Only)
```json
{
  "title": "Knight with Sword"
}
```

### Update Minifigure (Partial)
```json
{
  "field_description": "Updated description"
}
```

## Response Examples

### Success Response (GET/POST/PUT)
```json
{
  "id": 1,
  "title": "Classic Space Astronaut",
  "field_name": "Space Explorer",
  "field_description": "A classic blue space astronaut minifigure",
  "field_image": {
    "fid": 5,
    "filename": "astronaut.jpg",
    "uri": "public://astronaut.jpg",
    "url": "http://localhost/drupal/sites/default/files/astronaut.jpg"
  },
  "created": 1234567890,
  "updated": 1234567890
}
```

### List Response (GET index)
```json
{
  "data": [
    {
      "id": 1,
      "title": "Minifigure 1",
      ...
    },
    {
      "id": 2,
      "title": "Minifigure 2",
      ...
    }
  ],
  "count": 2,
  "offset": 0,
  "limit": 25
}
```

### Delete Response
```json
{
  "status": "deleted",
  "id": 1,
  "title": "Classic Space Astronaut"
}
```

## Authentication Workflow

### Recommended: Session-Based Authentication

1. **Login:**
   - Use "POST - User Login" from Authentication folder
   - Enter your `username` and `password` in collection variables
   - Send the request
   - Postman automatically:
     - Saves session cookies
     - Extracts and saves `session_token` (CSRF token)
     - Saves `user_id` and `user_name`

2. **Use Authenticated Endpoints:**
   - All subsequent requests automatically use saved cookies
   - CSRF token is automatically added to headers where needed
   - You're now authenticated for all minifigure operations

3. **Logout (Optional):**
   - Use "POST - User Logout" when done
   - Clears the session

### Alternative: Basic Authentication
- Go to request → Authorization tab
- Select "Basic Auth"
- Enter username and password
- Note: This doesn't work with all Drupal Services configurations

## Troubleshooting

### 404 Not Found
- Check that your Services endpoint is configured correctly
- Verify the endpoint path (should be `/api/v1` or your custom path)
- Ensure the `lego_minifigures` resource is enabled in Services

### 403 Forbidden
- Check user permissions
- Ensure user has "access content" permission
- For create/update/delete, ensure proper node permissions

### 400 Bad Request
- Verify JSON syntax in request body
- Check that required fields are provided
- Ensure `title` field is present for create operations

### Empty Response
- Check if Content Type "Minifigure" exists
- Verify fields `field_name`, `field_description`, `field_image` exist
- Clear Drupal cache

## Tips

1. **Save Responses:** After creating a minifigure, copy the `id` from response and update the `minifigure_id` variable for easy testing

2. **Test Scripts:** Add this to "Tests" tab to auto-update variable:
   ```javascript
   if (pm.response.code === 200) {
       var jsonData = pm.response.json();
       if (jsonData.id) {
           pm.collectionVariables.set("minifigure_id", jsonData.id);
       }
   }
   ```

3. **Environment Variables:** Create a Postman Environment for different setups (local, staging, production)

4. **Collection Runner:** Use Collection Runner to test all endpoints in sequence

