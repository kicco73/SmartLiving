<?php
class Resource_model extends CI_Model {
	var $fields = array(
			//'id',
			'name', 'description', 'resType', 'unit', 'currentValue', 'timestamp'
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
		
		
	function list_all_samples($id) {
		return $this->db->where("resourceId", $id)->get("Sample")->result();
	}
	
	function add_sample($name, $value) {
		$result = $this->db->where('name', $name)->get('Resource')->result();
		if(count($result) > 0) {
			$resource = $result[0];
			$resource->currentValue = $value;
			$this->update($resource);
			$this->db->insert('Sample', array('resourceId' => $resource->id, 'value' => $value));
		}
	}

	function insert_or_update_by_name($name, $value, $unit, $type) {
		$result = $this->db->where('name', $name)->get('Resource')->result();
		if(count($result) > 0) {
			$resource = $result[0];
			$resource->currentValue = $value;
			$resource->unit = $unit;
			$resource->resType = $type;
			$this->update($resource);
		} else
			$this->db->insert('Resource', 
				array('name' => $name, 'currentValue' => $value, 
					  'unit' => $unit, 'resType' => $type));
	}
	
	function delete($id) {
		$this->db->where('id', $id)->delete('Resource'); 
	}
	
}
