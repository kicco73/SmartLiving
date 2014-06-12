<?php
class Fake_model extends CI_Model {
	var $fields = array(
			//'id',
			'url', 'it', 'en', 'product'
	);
		
	function __construct() {
		parent::__construct();
	}
	
	function get_default() {
		$this->db->where('product', 'basic')->limit(1);
		$result = $this->db->get('skin')->result();
		if($result)
			return $result[0];
	}

	function get($skinid) {
		$this->db->where('id', $skinid);
		$result = $this->db->get('skin')->result();
		if($result)
			return $result[0];
	}
	
	function list_all() {
		// FIXME order_by hardcoded
		$skins = $this->db->get("Ciao")->result();
		return $skins;
	}
	
}
