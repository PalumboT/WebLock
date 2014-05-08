<?php

	/**
	 * Use by the WebLock server to access a MySQL database
	 * Require the class.MySQL.php v0.8.1 (https://github.com/a1phanumeric/PHP-MySQL-Class/blob/master/class.MySQL.php)
	*/

	require('class.MySQL.php');

	class db {
		
		var $database;

		function __construct(){
			$config = parse_ini_file(ROOT_PATH."/config.ini", true)['database'];
			$this->database = new MySQL($config['database'], $config['username'], $config['password']);
		}

		function getUsers(){
			return $this->database->select('users');
		}

	}

?>