<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Resources extends CI_Controller {

	const IO_TIMEOUT = 10.0;	// Seconds to wait before network timeout
	
	private function __rpc($url, $method="GET", $user_agent="Mozilla (5.0)") {
		$info = false;
		$opts = array(
  			'http'=>array(
				'timeout' => Resources::IO_TIMEOUT,
 			    'method'=>$method,
 			    'header'=> "User-agent: $user_agent"
 			 )
		);
		$context = stream_context_create($opts);
		$stream = @fopen($url, "r", false, $context);
		if(is_resource($stream)) {
			$info = @stream_get_contents($stream, 1024);
			if($info == false)
				$info = "";
			fclose($stream);
		} else 
			log_message('error', $url . ': stream is not resource');

		if ($info != false && mb_detect_encoding($info, 'UTF-8', true) === false)
    			$info = utf8_encode($info);
  		return $info;
	}

	private function _restPut($path, $value) {
		return $this->__rpc($path."?v=".$value, "PUT") !== false;
	}
	
	private function _restGet($path) {
		$rv = $this->__rpc($path);
		return $rv == false? false : json_decode($rv);
	}

	public function index() {
		$resources = $this->Resource_model->list_all();
		$this->load->view('list_resources', array("resources" => $resources));
	}

	public function ajaxSwitchToggle($id) {
		$resource = $this->Resource_model->get($id);
		$resource->currentValue = $resource->currentValue > 0? 0 : 1;
		if($this->_restPut($resource->name, $resource->currentValue)) {
			$this->Resource_model->update($resource);
			$this->output->set_content_type('application/json')->set_output(json_encode($resource));
		} else {
			log_message('error', 'resource directory unreachable');
			$this->output->set_status_header('500', 'resource directory unreachable');
		}
	}

	public function ajaxDimmerChange($id, $val) {
		$resource = $this->Resource_model->get($id);
		$resource->currentValue = (float) $val;
		$restUrl = $resource->name+"?v=".$resource->currentValue;
		if($this->_restPut($resource->name, $resource->currentValue)) {
			$this->Resource_model->update($resource);
			$this->output->set_content_type('application/json')->set_output(json_encode($resource));
		} else {
			log_message('error', 'resource directory unreachable');
			$this->output->set_status_header('500', 'resource directory unreachable');
		}
	}
	
	public function ajaxSensorRefresh($id) {
		$resource = $this->Resource_model->get($id);
		$rv = $this->_restGet($resource->name);
		if($rv != false) {
			$resource->currentValue = $rv->v;
			$this->Resource_model->update($resource);
			$resource = $this->Resource_model->get($resource->id);
			$this->output->set_content_type('application/json')->set_output(json_encode($resource));
		} else {
			log_message('error', 'resource directory unreachable');
			$this->output->set_status_header('500', 'resource directory unreachable');
		}
	}
	
	public function ajaxResourceDirectoryRefresh() {
		$rv = $this->_restGet($this->_rd_baseurl()."/");
		if($rv != false) {
			foreach($rv as $r) 
				$this->Resource_model->insert_or_update_by_name($r->n, $r->v, $r->u, $r->rt);
			$resources = $this->Resource_model->list_all();
			$this->load->view('ajax_list_resources', array("resources" => $resources));
		} else {
			log_message('error', 'resource directory unreachable');
			$this->output->set_status_header('500', 'resource directory unreachable');
		}
	}
	
	public function onUpdate() {
		$method = strtolower($this->input->server('REQUEST_METHOD'));
		switch($method) {
		case 'post':
			$input_data = trim(file_get_contents('php://input'));
			log_message('error', 'input data '.$input_data);
			$input_data = json_decode($input_data);
			foreach($input_data as $r)
				$this->Resource_model->add_sample($r->n, $r->v);
			$this->output->set_status_header('204'); // no content
			break;
		default:
			$this->output->set_status_header('400'); // bad request
			break;
		}
	}
	
	public function ajaxResourceRemove($id) {
		$this->Resource_model->delete($id);
		$this->output->set_status_header('204'); // no content	
	}
	
	private function _rd_baseurl() {
		return $this->config->base_url()."resources/fake_rd";
	}
	
	public function fake_rd() {
		$method = strtolower($this->input->server('REQUEST_METHOD'));
		$b = $this->_rd_baseurl();
		switch($method) {
		case 'put':
			$value = (float) $this->input->get('v', TRUE);
			$this->output->set_status_header('204'); // no content
			break;
		case 'get':
			$n = "/".join("/", func_get_args());
			if($n == "/") {
				$rv = array(
					array("n" => $b."/fan", "v" => 0.0, "u" => "", "rt" => "switch"),
					array("n" => $b."/light", "v" => 0.0, "u" => "lux", "rt" => "dimmer"),
					array("n" => $b."/accelerometer", "v" => 0.0, "u" => "m/s^2", "rt" => "sensor")
				);
			} else 
				$rv = array("n" => $b.$n, "v" => 0.5);
			$this->output->set_content_type('application/json')
						 ->set_status_header('200') // ok
						 ->set_output(json_encode($rv));
			break;
		default:
			$this->output->set_status_header('400'); // bad request
			break;
		}
	}
}

/* End of file resource.php */
/* Location: ./application/controllers/resource.php */
