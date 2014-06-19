<?php
class Directory_model extends CI_Model {

	const TABLENAME = "Directory";
	
	var $fields = array(
			//'id',
			'url', 'name', 'description'
	);

	function get($id) {
		$result = $this->db->where('id', $id)->get(Directory_model::TABLENAME)->result();
		return $result[0];		
	}
	
	function update($resource) {
		$this->db->where('id', $resource->id)->update(Directory_model::TABLENAME, $resource);
	}
	
	function list_all() {
		return $this->db->get(Directory_model::TABLENAME)->result();
	}
		
	function insert_or_update_by_url($url) {
		$result = $this->db->where('url', $url)->get(Directory_model::TABLENAME)->result();
		if(count($result) > 0) {
			$entry = $result[0];
			$this->update($entry);
		} else
			$this->db->insert(Directory_model::TABLENAME, array('url' => $url));
	}
	
	function delete($id) {
		$this->db->where('id', $id)->delete(Directory_model::TABLENAME); 
	}
	
}
