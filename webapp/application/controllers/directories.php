<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');

class Directories extends CI_Controller {

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

	private function __coap_rpc($url, $method="GET", $value=null, $timeout=2) {
		$info = false;
		$descriptorspec = array(
  			0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
   			1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
   			2 => array("file", "/dev/null", "a") // stderr is a file to write to
		);
		$process = proc_open(BASEPATH."../bin/coap-client -b0 -v0 -B".$timeout." -f- -m ".$method.' "'.$url.'"', $descriptorspec, $pipes);
		if($value != null)
			fwrite($pipes[0], $value);
		fclose($pipes[0]);
		$lines = stream_get_contents($pipes[1]);
		fclose($pipes[1]);
		proc_close($process);
		$info = trim($lines);
		if ($info != false && mb_detect_encoding($info, 'UTF-8', true) === false)
    			$info = utf8_encode($info);
  		return $info;
	}

	private function _restPut($path, $value) {
		$rv = strpos($path, "coap:") == 0? $this->__coap_rpc($path, "PUT", "v=".$value) : $this->__http_rpc($path."?v=".$value, "PUT");
		return $rv !== false;
	}
	
	private function _restGet($path, $timeout) {
		$rv = strpos($path, "coap:") == 0? $this->__coap_rpc($path, "GET", null, $timeout) : $this->__http_rpc($path);
		if($rv == false)
			return false;
		log_message("error", "RESTGET: ".$rv);
		$rv = json_decode($rv);
		if(is_numeric($rv)) {
			$v = $rv;
			$rv = new stdClass();
			$rv->v = $v;
		}
		return $rv;
	}

	public function index() {
		$directories = $this->Directory_model->list_all();
		$this->load->view('list_directories', array("directories" => $directories));
	}
	
	public function ajaxRebuild($id) {
		$directory = $this->Directory_model->get($id);
		$b = $directory->url;
		$rv = $this->_restGet($b."/list", 10);
		if($rv != false) {
			foreach($rv as $r) {
				$board = $this->Board_model->insert_or_update_by_directory_and_url($id, $r->a);
				$this->Resource_model->insert_or_update_by_board_and_url($board->id, 'coap://['.$r->a.']'.$r->n, $r->v, $r->u, $r->rt);
			}
			$boards = $this->Board_model->list_by_directory($id);
			$this->load->view('ajax_list_resources', array("boards" => $boards));
		} else {
			log_message('error', 'resource directory unreachable');
			$this->output->set_status_header('500', 'resource directory unreachable');
		}
	}
	
	public function ajaxRefresh($id) {
		$boards = $this->Board_model->list_by_directory($id);
		$this->load->view('ajax_list_resources', array("boards" => $boards));
	}
	
	public function ajaxRemove($id) {
		$directory = $this->Directory_model->get($id);
		$this->Directory_model->delete($id);
		$this->output->set_status_header('204'); // no content	
	}
	
	public function ajaxDiscover() {
		$descriptorspec = array(
  			0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
   			1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
   			2 => array("pipe", "w") // stderr is a file to write to
		);
		$process = proc_open(BASEPATH."../bin/coap-client -v7 -B2 -m GET coap://[ff02::1%tun0]/.well-known/core", $descriptorspec, $pipes);
		fclose($pipes[0]);
		$lines = stream_get_contents($pipes[2]);
		fclose($pipes[1]);
		fclose($pipes[2]);
		proc_close($process);
		log_message("error", "XXX GUARDA CHE HO: ".$lines);
		$lines = explode("\n", $lines);
		foreach($lines as $line) {
			log_message("error", "GUARDA CHE HO: ".$line);
			if(preg_match("/bytes from \\[(.+?)\\]/", $line, $matches)) {
				$url = "coap://[".$matches[1]."]";
				$this->Directory_model->insert_or_update_by_url($url);
			}
		}
		$this->output->set_status_header('204');
	}

	public function url_check($input) {
		return $ok;
	}
	
	public function ajaxAdd() {
		$url = $this->input->post('url', TRUE);
		$ok = preg_match("/^(http|coap):\/(\/[^\/]+)+$/i", $url, $matches);
		if(!$ok) {
			$this->output->set_status_header('500', "field must be in the form {http|coap}://hostname/path");
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
					array("n" => "coap://localhost/fan", "v" => 0.0, "u" => "", "rt" => "switch", "a" => "localhost"),
					array("n" => "coap://localhost/light", "v" => 0.0, "u" => "lux", "rt" => "dimmer", "a" => "localhost"),
					array("n" => "coap://localhost/accelerometer", "v" => 0.0, "u" => "m/s^2", "rt" => "sensor", "a" => "localhost")
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
	
	public function fake_board($id) {
	}
}

/* End of file resource.php */
/* Location: ./application/controllers/resource.php */
