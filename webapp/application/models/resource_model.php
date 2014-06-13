<?php
class Resource_model extends CI_Model {
	var $fields = array(
			//'id',
			'name', 'description', 'resType', 'unit', 'currentValue'
	);

	function get($id) {
		$result = $this->db->where('id', $id)->get('Resource')->result();
		return $result[0];		
	}
	
	function update($resource) {
		$this->db->where('id', $resource->id)->update('Resource', $resource);
	}
	
	function list_all() {
		return $this->db->get("Resource")->result();
	}
	
}
