<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Resource extends CI_Controller {

	const IO_TIMEOUT = 10.0;	// Seconds to wait before network timeout
	
	function __rpc($url, $method="GET", $user_agent="Mozilla (5.0)") {
		$info = false;
		$opts = array(
  			'http'=>array(
				'timeout' => Resource::IO_TIMEOUT,
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

	function _restPut($path, $value) {
		$path = $this->_rd_baseurl() . $path;
		return $this->__rpc($path."?v=".$value, "PUT") !== false;
	}
	
	function _restGet($path) {
		$path = $this->_rd_baseurl() . $path;
		$rv = $this->__rpc($path);
		return $rv == false? false : json_decode($rv);
	}

	public function index() {
		$resources = $this->Resource_model->list_all();
		$this->load->view('list_resources', array("resources" => $resources));
	}

	public function json_switch_toggle($id) {
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

	public function json_dimmer_change($id, $val) {
		$resource = $this->Resource_model->get($id);
		$resource->currentValue = (float) $val;
		$restUrl = $this->_rd_baseurl().$resource->name+"?v=".$resource->currentValue;
		if($this->_restPut($resource->name, $resource->currentValue)) {
			$this->Resource_model->update($resource);
			$this->output->set_content_type('application/json')->set_output(json_encode($resource));
		} else {
			log_message('error', 'resource directory unreachable');
			$this->output->set_status_header('500', 'resource directory unreachable');
		}
	}
	
	public function json_sensor_refresh($id) {
		$resource = $this->Resource_model->get($id);
		$this->output->set_content_type('application/json')->set_output(json_encode($resource));
	}
	
	function _rd_baseurl() {
		return $this->config->base_url()."index.php/resource/fake_rd";
	}
	
	public function fake_rd() {
		$method = strtolower($this->input->server('REQUEST_METHOD'));
		switch($method) {
		case 'put':
			$value = (float) $this->input->get('v', TRUE);
			$this->output->set_status_header('204'); // no content
			break;
		case 'get':
			$n = "/".join("/", func_get_args());
			$rv = array("n" => $n, "v" => 0.5);
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
