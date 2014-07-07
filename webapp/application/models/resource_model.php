<?php
class Resource_model extends CI_Model {

	const TABLENAME = "Resource";
	
	var $fields = array(
			//'id',
			'url', 'name', 'description', 'resType', 'unit', 'currentValue', 'timestamp', 'boardId'
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
		
	function list_by_board($boardId) {
		return $this->db->where("boardId", $boardId)->get(Resource_model::TABLENAME)->result();
	}
		
	function list_by_directory($directoryId) {
		return $this->db->join("Board", "Board.id = boardId")->where("directoryId", $directoryId)->get(Resource_model::TABLENAME)->result();
	}
		
	function list_all_samples($id) {
		return $this->db->where("resourceId", $id)->get("Sample")->result();
	}
	
	function add_sample($url, $value) {
		$result = $this->db->like('url', $url, 'before')->get(Resource_model::TABLENAME)->result();
		if(count($result) > 0) {
			$resource = $result[0];
			$resource->currentValue = $value;
			$this->update($resource);
			$this->db->insert('Sample', array('resourceId' => $resource->id, 'value' => $value));
		} else log_message("error", "resource_model.add_sample(): ".$url.": cannot find resource");
		return count($result) > 0;
	}

	function count_samples($id) {
		return $this->db->where('resourceId', $id)->count_all_results("Sample");
	}

	function insert_or_update_by_board_and_url($boardId, $url, $value, $unit, $type) {
		$result = $this->db->where('url', $url)->get(Resource_model::TABLENAME)->result();
		if(count($result) > 0) {
			$resource = $result[0];
			$resource->currentValue = $value;
			$resource->unit = $unit;
			$resource->resType = $type;
			$resource->boardId = $boardId;
			$this->update($resource);
		} else
			$this->db->insert(Resource_model::TABLENAME, 
				array('boardId' => $boardId,
					  'url' => $url, 'currentValue' => $value, 
					  'unit' => $unit, 'resType' => $type));
	}

	function delete($id) {
		$this->db->where('id', $id)->delete(Resource_model::TABLENAME); 
	}
	
}
