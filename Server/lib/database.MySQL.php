<?php

	/**
	 * Use by the WebLock server to access a MySQL database
	 * Require the class.MySQL.php v0.8.1 (https://github.com/a1phanumeric/PHP-MySQL-Class/blob/master/class.MySQL.php)
	*/

	require('class.MySQL.php');

	class db {

		
		var $database;	// Database Connection Link

	   	/* *******************
	 	 * Class Constructor *
	 	 * *******************/

		function __construct(){
			// Get the configuration file that contain the login data for the database
			$config = parse_ini_file(ROOT_PATH."/config.ini", true)['database'];
			// Open a connetion with the database
			$this->database = new MySQL($config['database'], $config['username'], $config['password']);
		}

		/* *********************************************
		 * Return an array that contain the users list *
		 * *********************************************/

		function getUser($key){
			return $this->database->select('users', array('idkey' => $key), '', '', false, 'AND', '*', array('string'));
		}

		/* **********************
		 * Users CRUD functions *
		 * **********************/

		function getUsers(){
			return $this->database->select('users', '', '', '', false, 'AND', '*', '');
		}

		function addUser($user){
			return $this->database->insert('users', $user, '', array('string','string','string', 'int'));
		}

		function updateUser($id, $user){
			return $this->database->update('users', $user, array('iduser'=>$id), '', array('string', 'string', 'string', 'int'), array('int'));
		}

		function deleteUser($id){
			return $this->database->delete('users', array('iduser'=>$id), '', false, array('int'));
		}

		/***********************
		 * History functions   *
		 ***********************/

		function getHistory($begin = null, $end = null){

			$beginFinal = '';
			$endFinal = '';

			if(isset($begin) || isset($end)){
				$validDateBegin = $this->validateDate($begin, 'm/d/Y');
				$validDateEnd = $this->validateDate($end, 'm/d/Y');
				if(($validDateBegin == 1) || ($validDateBegin == 1)){
					$beginFinal = date('Y-m-d H:i:s', strtotime($begin));
					$endFinal = date('Y-m-d H:i:s', strtotime($end));
					return $this->database->executeSQL("SELECT * FROM accesslog WHERE date BETWEEN '$beginFinal' AND '$endFinal' ORDER BY date DESC");
				}
			}

			return $this->database->select('accesslog', $where, 'date DESC', '', false, 'AND', '*', $whereTypes);
		}

		/********************
		 * Stats function   *
		 ********************/

		function getStatResult($query){
			$result = $this->database->executeSQL($query);
			if($result) return array_values($result[0])[0];
			return null;
		}


		/********************
		 * Date validating  *
		 ********************/

		function validateDate($date, $format = 'Y-m-d H:i:s')
		{
		    $d = DateTime::createFromFormat($format, $date);
		    return $d && $d->format($format) == $date;
		}

		/* ***************************************
		 * Create the access log in the database *
		 * ***************************************/

		function addAccess($key, $user = null){

			// This object will contain all the data of the log
			$access = array();
			// Set the date of access (now)
			$access['date'] = date('Y-m-d H:i:s');
			// If a user has the key, $user will contain the user information
			if($user) {
				// We don't need the user id, we remove it from the user table
				unset($user['iduser']);
				// Merge the array that contain the user data and the access log that contain the date
				$access = array_merge($user, $access);
				// Insertion of the log in the database
				$this->database->insert('accesslog', $access, '', array('string','string','string', 'int', 'datetime', 'boolean'));
			// Else if no user have this key assigned 
			} else{
				// Set isAllowed to 0
				$access['permission'] = 0;
				// Set the key in the data to insert. We didn't do that before becouse the key id is already present in the data of the user.
				$access['idkey'] = $key;
				// Insertion of the log in the database
				return $this->database->insert('accesslog', $access, '', array('datetime','boolean','string'));
			}
			
		}

	}

?>