<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Directories extends CI_Controller {

	const IO_TIMEOUT = 10.0;	// Seconds to wait before network timeout
	
	private function __rpc($url, $method="GET", $user_agent="Mozilla (5.0)") {
		$info = false;
		$opts = array(
  			'http'=>array(
				'timeout' => Directories::IO_TIMEOUT,
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
		$directories = $this->Directory_model->list_all();
		$this->load->view('list_directories', array("directories" => $directories));
	}
	
	public function ajaxRebuild($id) {
		$directory = $this->Directory_model->get($id);
		$b = $directory->url;
		$rv = $this->_restGet($b."/");
		if($rv != false) {
			foreach($rv as $r) 
				$this->Resource_model->insert_or_update_by_url($id, $r->n, $r->v, $r->u, $r->rt);
			$resources = $this->Resource_model->list_by_directory($id);
			$this->load->view('ajax_list_resources', array("resources" => $resources));
		} else {
			log_message('error', 'resource directory unreachable');
			$this->output->set_status_header('500', 'resource directory unreachable');
		}
	}
	
	public function ajaxRefresh($id) {
		$resources = $this->Resource_model->list_by_directory($id);
		$this->load->view('ajax_list_resources', array("resources" => $resources));
	}
	
	public function ajaxRemove($id) {
		$directory = $this->Directory_model->get($id);
		$this->Directory_model->delete($id);
		$this->output->set_status_header('204'); // no content	
	}
	
	public function ajaxDiscover() {
		$output = shell_exec("/home/user/SmartLiving/libcoap-4.1.1/examples/coap-client -B2 -m GET coap://[ff02::1]/.well-known/core");
		$this->output->set_status_header('500', 'tutto finto');
		$this->output
			->set_header("Content-Length: ".strlen($output))
			->set_output($output);
	}

	public function url_check($input) {
		return $ok;
	}
	
	public function ajaxAdd() {
		$url = $this->input->post('url', TRUE);
		$ok = preg_match("/^http:\/(\/[^\/]+)+$/i", $url, $matches);
		if(!$ok) {
			$this->output->set_status_header('500', "field must be in the form http://hostname/path");
		} else {
			$id = $this->Directory_model->insert_or_update_by_url($url);
			$this->ajaxRefresh($id);
		}
	}
	
	public function ajaxSet($id) {
		$field = $this->input->post('field', TRUE);
		$value = $this->input->post('value', TRUE);
		$resource = $this->Resource_model->get($id);
		$resource->$field = $value;
		$this->Resource_model->update($resource);
		$this->output->set_status_header('204'); // no content	
	}

	public function fake_rd() {
		$method = strtoupper($this->input->server('REQUEST_METHOD'));
		switch($method) {
		case 'PUT':
			// Update resource value
			$value = (float) $this->input->get('v', TRUE);
			$this->output->set_status_header('204'); // no content
			break;
		case 'GET':
			$n = "/".join("/", func_get_args());
			if($n == "/") {
				// Read whole resource directory
				$rv = array(
					array("n" => "/fan", "v" => 0.0, "u" => "", "rt" => "switch"),
					array("n" => "/light", "v" => 0.0, "u" => "lux", "rt" => "dimmer"),
					array("n" => "/accelerometer", "v" => 0.0, "u" => "m/s^2", "rt" => "sensor")
				);
			} else 
				// Read just one value
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
