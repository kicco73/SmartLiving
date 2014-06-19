<?php
class Resource_model extends CI_Model {

	const TABLENAME = "Resource";
	
	var $fields = array(
			//'id',
			'url', 'name', 'description', 'resType', 'unit', 'currentValue', 'timestamp'
	);

	function get($id) {
		$result = $this->db->where('id', $id)->get(Resource_model::TABLENAME)->result();
		return $result[0];		
	}
	
	function update($resource) {
		$this->db->where('id', $resource->id)->update(Resource_model::TABLENAME, $resource);
	}
	
	function list_all() {
		return $this->db->get(Resource_model::TABLENAME)->result();
	}
		
	function list_by_directory($directory_id) {
		return $this->db->get(Resource_model::TABLENAME)->result();
	}
		
	function list_all_samples($id) {
		return $this->db->where("resourceId", $id)->get("Sample")->result();
	}
	
	function add_sample($url, $value) {
		$result = $this->db->where('url', $url)->get(Resource_model::TABLENAME)->result();
		if(count($result) > 0) {
			$resource = $result[0];
			$resource->currentValue = $value;
			$this->update($resource);
			$this->db->insert('Sample', array('resourceId' => $resource->id, 'value' => $value));
		}
	}

	function insert_or_update_by_url($url, $value, $unit, $type) {
		$result = $this->db->where('url', $url)->get(Resource_model::TABLENAME)->result();
		if(count($result) > 0) {
			$resource = $result[0];
			$resource->currentValue = $value;
			$resource->unit = $unit;
			$resource->resType = $type;
			$this->update($resource);
		} else
			$this->db->insert(Resource_model::TABLENAME, 
				array('url' => $url, 'currentValue' => $value, 
					  'unit' => $unit, 'resType' => $type));
	}
	
	function delete($id) {
		$this->db->where('id', $id)->delete(Resource_model::TABLENAME); 
	}
	
}
