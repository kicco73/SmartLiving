<?php
class Board_model extends CI_Model {

	const TABLENAME = "Board";
	
	var $fields = array(
			//'id',
			'url', 'name', 'description', 'timestamp', 'directoryId'
	);

	function get($id) {
		$result = $this->db->where('id', $id)->get(Board_model::TABLENAME)->result();
		return $result[0];		
	}
	
	function update($resource) {
		$this->db->where('id', $resource->id)->update(Board_model::TABLENAME, $resource);
	}
	
	function list_all() {
		return $this->db->get(Board_model::TABLENAME)->result();
	}
		
	function get_by_url($url) {
		$result = $this->db->where('url', $url)->get(Board_model::TABLENAME)->result();
		return $result[0];		
	}
	
	function insert_or_update_by_directory_and_url($directoryId, $url) {
		$result = $this->db->where('url', $url)->get(Board_model::TABLENAME)->result();
		if(count($result) > 0) {
			$board = $result[0];
			$board->directoryId = $directoryId;
			$this->update($board);
			return $result[0];		
		}
		$this->db->insert(Board_model::TABLENAME, 
			array('directoryId' => $directoryId, 'url' => $url));
		return $this->get_by_url($url);
	}
	
	function list_by_directory($directoryId) {
		return $this->db->where("directoryId", $directoryId)->get(Board_model::TABLENAME)->result();
	}
		
	function delete($id) {
		$this->db->where('id', $id)->delete(Board_model::TABLENAME); 
	}
	
}
