<?php
	
	/*
	 * 	Timothée Palumbo (https://github.com/PalumboT)
	 * 
	 *  WebLock by WebForge is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
	 *	Based on a work at https://github.com/PalumboT/WebLock.
	*/
	
    // If the user is not authentified, redirect to the login file
	if (!isset($_SERVER['PHP_AUTH_DIGEST'])) header("Location:index.php");

    // Define the main directory path
	define('ROOT_PATH', realpath(__DIR__)."/../..");
	
	// Include the database manager file
	require_once(ROOT_PATH.'/lib/database.MySQL.php');

	/*
     * Instancation of the database class
	*/

	$db = new db();

	/*
	 * Get the action in the URL
	*/

	$action = $_GET['action'];

	/*
	 * Will set to true if error
	*/

	$error = null;

	/*
	 * Switch on the action to determine what we have to do
	*/

	switch($action) {
	    case 'create':
	    	$status = create();
	        break;

	    case 'update':
	    	$status = update();
	        break;

	    case 'delete':
	    	$status = delete();
	        break;

	    default:
	        break;
	}


	/*
     * Update the offline file
	*/

	if(!$error){
		$users = $db->getUsers();
		foreach($users as $user) {
			$out .= "# $user[firstname] $user[lastname] \n $user[idkey] $user[permission] \n";
		}
		file_put_contents(ROOT_PATH."/../files/keys.txt", $out);
	}


	/*
     * Redirect to the main page
	*/

	header('location:../manage.php?status='.$status);

	/********************
	 * Functions    	*
	 ********************/

	function create(){

		// Referencing the database class variable as global
		global $db, $error;

		// Get the values of the form
		$user = array('firstname' => $_POST['firstname'], 'lastname' => $_POST['lastname'], 'idkey' => $_POST['idkey'], 'permission' => $_POST['permission']);

		// check if the key doesn't already exist
		if($_POST['idkey'] !== ''){
			// If an user have the key, return an error and do nothing else
			if(is_array($db->getUser($_POST['idkey']))) {
				$error = true;
				return 'errorKey';
			}
		}

		// If the permission is not set (when you create a key with the history table for exemple)
		// set the permission as denied
		if($_POST['permission'] !== ''){
			$user['permission'] = 2;
		}

     	// Add the user in the database
		$db->addUser($user);

		// Return a confirmation message
		return "createOk";

	}

	function update(){

		// Referencing the database class variable as global
		global $db, $error;
		
		// Get the id of the user
		$id = $_GET['id'];

		// Get the values of the form
		$user = array('firstname' => $_POST['firstname'], 'lastname' => $_POST['lastname'], 'idkey' => $_POST['idkey'], 'permission' => $_POST['permission']);

		// check if the key doesn't already exist
		if($_POST['idkey'] !== ''){
			// Select all user in the database with hold the key
			$users = $db->getUser($_POST['idkey']);
			// If there is a single user that old the key
			if(is_array($users)){
				// If the user that hold the key isn't the same user of the update
				if($users[0]['iduser'] != $id){
					$error = true;
					// return an error and do nothing else
					return 'errorKey';
				}
			}
		}

     	// Add the user in the database
		$db->updateUser($id, $user);

		// Return a confirmation message
		return "updateOk";
	}

	function delete(){

		// Referencing the database class variable as global
		global $db;
		
		// Get the id of the user
		$id = $_GET['iduser'];

		// Delete the user form the database
		$db->deleteUser($id);

		// Return a confirmation message
		return "deleteOk";

	}
	
?>