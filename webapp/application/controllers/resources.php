<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Resources extends CI_Controller {

	const IO_TIMEOUT = 10.0;	// Seconds to wait before network timeout

	private function __http_rpc($url, $method="GET", $user_agent="Mozilla (5.0)") {
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

	private function __coap_rpc($url, $method="GET") {
		$info = false;
		exec(BASEPATH."../bin/coap-client -v0 -B2 -m ".$method.' "'.$url.'"', $lines);

		foreach($lines as $line) {
			if(!preg_match("/^v:/", $line, $matches)) {
				$info = $line;
			} else $info = "";
		}

		if ($info != false && mb_detect_encoding($info, 'UTF-8', true) === false)
    			$info = utf8_encode($info);
  		return $info;
	}

	private function _restPut($path, $value) {
		$rv = strpos($path, "coap:") == 0? $this->__coap_rpc($path."?v=".$value, "PUT") : 
										  $this->__http_rpc($path."?v=".$value, "PUT");
		return $rv !== false;
	}
	
	private function _restGet($path) {
		$rv = strpos($path, "coap:") == 0? $this->__coap_rpc($path) :
										  $this->__http_rpc($path);
		if($rv == false)
			return false;
		$rv = json_decode($rv);
		if(is_numeric($rv))
			$rv = array('v' => $rv);
		return $rv;
	}

	public function index() {
		$resources = $this->Resource_model->list_all();
		$this->load->view('list_resources', array("resources" => $resources));
	}

	private function getUrl($resource) {
		$url = $resource->url;
		if(strpos($url, "http:") !== 0 && strpos($url, "coap:") !== 0) {
			$board = $this->Board_model->get($resource->boardId);
			$directory = $this->Directory_model->get($board->directoryId);
			$url = $directory->url.$url;
		} 
		return $url;
	}

	public function ajaxSwitchToggle($id) {
		$resource = $this->Resource_model->get($id);
		$url = $this->getUrl($resource);
		$rv = $this->_restGet($url);
		$resource->currentValue = $resource->currentValue > 0? 0 : 1;
		if($this->_restPut($url, $resource->currentValue)) {
			$this->Resource_model->update($resource);
			$this->output->set_content_type('application/json')->set_output(json_encode($resource));
		} else {
			log_message('error', 'resource directory unreachable');
			$this->output->set_status_header('500', $resource->url.': resource directory unreachable');
		}
	}

	public function ajaxDimmerChange($id, $val) {
		$resource = $this->Resource_model->get($id);
		$url = $this->getUrl($resource);
		$resource->currentValue = (float) $val;
		$restUrl = $url+"?v=".$resource->currentValue;
		if($this->_restPut($url, $resource->currentValue)) {
			$this->Resource_model->update($resource);
			$this->output->set_content_type('application/json')->set_output(json_encode($resource));
		} else {
			log_message('error', 'resource directory unreachable');
			$this->output->set_status_header('500', $resource->url.': resource directory unreachable');
		}
	}
	
	public function ajaxSensorRefresh($id) {
		$resource = $this->Resource_model->get($id);
		$url = $this->getUrl($resource);
		$rv = $this->_restGet($url);
		if($rv != false) {
			$resource->currentValue = $rv->v;
			$this->Resource_model->update($resource);
			$resource = $this->Resource_model->get($resource->id);
			$this->output->set_content_type('application/json')->set_output(json_encode($resource));
		} else {
			log_message('error', $url.'resource directory unreachable');
			$this->output->set_status_header('500', $url.': resource directory unreachable');
		}
	}
	
	public function onUpdate() {
		$method = strtolower($this->input->server('REQUEST_METHOD'));
		switch($method) {
		case 'post':
			$input_data = trim(file_get_contents('php://input'));
			//log_message('error', 'input data '.$input_data);
			$input_data = json_decode($input_data);
			if($input_data) {
				foreach($input_data as $r) {
					$ok = $this->Resource_model->add_sample($r->n, $r->v);
					if(!$ok) {
						$this->output->set_status_header('404', $r->n.': resource not found'); // not found
						return;
					}
				}
				$this->output->set_status_header('204'); // no content
			} else
				$this->output->set_status_header('400', "empty input"); // no content
			break;
		default:
			$this->output->set_status_header('400', 'can only process POST requests'); // bad request
			break;
		}
	}
	
	public function ajaxRemove($id) {
		$this->Resource_model->delete($id);
		$this->output->set_status_header('204'); // no content	
	}
	
	public function ajaxSet($id) {
		$field = $this->input->post('field', TRUE);
		$value = $this->input->post('value', TRUE);
		$resource = $this->Resource_model->get($id);
		$resource->$field = $value;
		$this->Resource_model->update($resource);
		$this->output->set_status_header('204'); // no content	
	}
	
	public function chart($id) {
		$resource = $this->Resource_model->get($id);
		$samples = $this->Resource_model->list_all_samples($id);
		$this->load->view("chart", array("resource" => $resource, "samples" =>$samples));
	}
	
}

/* End of file resource.php */
/* Location: ./application/controllers/resource.php */
