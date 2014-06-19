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
	
	function gethostbyname($host, $try_a = true) {
        // get AAAA record for $host
        // if $try_a is true, if AAAA fails, it tries for A
        // the first match found is returned
        // otherwise returns false

        $dns = gethostbynamel6($host, $try_a);
        if ($dns == false) { return false; }
        else { return $dns[0]; }
    }

    private function gethostbynamel6($host, $try_a = true) {
        // get AAAA records for $host,
        // if $try_a is true, if AAAA fails, it tries for A
        // results are returned in an array of ips found matching type
        // otherwise returns false

        $dns6 = dns_get_record($host, DNS_AAAA);
        if ($try_a == true) {
            $dns4 = dns_get_record($host, DNS_A);
            $dns = array_merge($dns4, $dns6);
        }
        else { $dns = $dns6; }
        $ip6 = array();
        $ip4 = array();
        foreach ($dns as $record) {
            if ($record["type"] == "A") {
                $ip4[] = $record["ip"];
            }
            if ($record["type"] == "AAAA") {
                $ip6[] = $record["ipv6"];
            }
        }
        if (count($ip6) < 1) {
            if ($try_a == true) {
                if (count($ip4) < 1) {
                    return false;
                }
                else {
                    return $ip4;
                }
            }
            else {
                return false;
            }
        }
        else {
            return $ip6;
        }
    }


}
