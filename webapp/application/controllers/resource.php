<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Resource extends CI_Controller {

	public function index() {
		$resources = $this->Resource_model->list_all();
		$this->load->view('list_resources', array("resources" => $resources));
	}

	public function json_switch_toggle($id) {
		$resource = $this->Resource_model->get($id);
		$resource->currentValue = $resource->currentValue > 0? 0 : 1;
		$this->Resource_model->update($resource);
		$this->output->set_content_type('application/json')->set_output(json_encode($resource));
	}

	public function json_dimmer_change($id, $val) {
		$resource = $this->Resource_model->get($id);
		$resource->currentValue = (float) $val;
		$this->Resource_model->update($resource);
		$this->output->set_content_type('application/json')->set_output(json_encode($resource));
	}
}

/* End of file resource.php */
/* Location: ./application/controllers/resource.php */
