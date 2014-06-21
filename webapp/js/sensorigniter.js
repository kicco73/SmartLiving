var observe = {
  selector: null,
  timer: null,
  period: 8,

  refresh: function() {
  	observe.pause();
    var n = $(observe.selector).length;
    $(observe.selector).each(function() {
    	var div = $(this);
		var rel = div.attr("rel");
		console.log("*** Observe: refreshing "+rel);
		$.ajax("/directories/ajaxRefresh/"+rel)
			.done(function(data) {
				div.closest(".directory").find(".resources").replaceWith(data);
			})
			.always(function(x) {
				n -= 1;
				if(n == 0)
					observe.run();
			});
    });
  },

  start: function(dirSelector) {
    observe.selector = dirSelector;
  	observe.run();
  },

  run: function() {
  	observe.pause();
  	observe.timer = window.setTimeout(observe.refresh, observe.period*1000);
	console.log("*** Observe: running");
  },

  pause: function() {
	if(observe.timer != null) {
        window.clearTimeout(observe.timer);
		observe.timer = null;
		console.log("*** Observe: paused");
	}
  }
};

var resource = {
  ajaxSwitchToggle: function() {
	var button = $(this);
	var rel = button.attr("rel");
	observe.pause();
	$.getJSON("/resources/ajaxSwitchToggle/"+rel)
	.done(function(data) {
		button.attr("class", "switch");
		button.addClass(data.currentValue? "on" : "off");
	}).fail(function() {
		alert("Cannot toggle switch");
	}).always(function() {
		observe.run();
	});
  },

  ajaxDimmerChange: function() {
	var dimmer = $(this);
	var rel = dimmer.attr("rel");
	var old = parseFloat(dimmer.attr("data"));
	var val = dimmer.slider("value");
	if(val != old) {
		observe.pause();
		$.getJSON("/resources/ajaxDimmerChange/"+rel+"/"+val)
		.done(function(data) {
			dimmer.attr("data", data.currentValue);
			dimmer.slider("value", data.currentValue);
		}).fail(function() {
			alert("Cannot change dimmer value");
		}).always(function() {
			observe.run();
		});
	}
  },

  ajaxSensorRefresh: function() {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	observe.pause();
	$.getJSON("/resources/ajaxSensorRefresh/"+rel)
	.done(function(data) {
		sensor.html(data.currentValue+" "+data.unit);
	}).fail(function() {
		alert("Cannot refresh sensor value");
	}).always(function() {
		observe.run();
	});
  },

  ajaxRemove: function() {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	observe.pause();
	$.getJSON("/resources/ajaxRemove/"+rel)
	.done(function(data) {
		sensor.closest('.resource').remove();
	}).fail(function() {
		alert("Cannot remove resource");
	}).always(function() {
		observe.run();
	});
  },
  ajaxSet: function(val, settings) {
	var div = $(this);
	var rel = div.attr("rel");
	var field = div.attr("data");
	observe.pause();
	$.post("/resources/ajaxSet/"+rel, {field: field, value: val}).fail(function() {
		alert("Cannot set "+field+" value");
	}).always(function() {
		observe.run();
	});
	return val;
  }
};

var directory = {
  ajaxRefresh: function() {
	var div = $(this);
	var rel = div.attr("rel");
	observe.pause();
	$.ajax("/directories/ajaxRefresh/"+rel)
		.done(function(data) {
			div.closest(".directory").find(".resources").replaceWith(data);
		}).fail(function(x) {
			alert("Cannot refresh resource directory");
		}).always(function() {
			observe.run();
		});
  },

  ajaxRemove: function() {
	var sensor = $(this);
	var rel = sensor.attr("rel");
	observe.pause();
	$.getJSON("/directories/ajaxRemove/"+rel)
	.done(function(data) {
		sensor.closest('.directory').remove();
	}).fail(function() {
		alert("Cannot remove resource directory");
	}).always(function() {
		observe.run();
	});
  },

  ajaxAdd: function() {
	var div = $(this);
	var url = div.closest(".directory").find("input.addDirectory").val();
	observe.pause();
	$.post("/directories/ajaxAdd", {url: url})
	.done(function(data) {
		window.location = window.location;
	}).fail(function() {
		alert("Must be in format http://hostname[/path]");
		observe.run();
	});
  },

  ajaxSet: function() {
	var div = $(this);
	var rel = div.attr("rel");
	var field = div.attr("data");
	observe.pause();
	$.post("/directories/ajaxSet/"+rel, {field: field, value: val})
	.fail(function() {
		alert("Cannot set "+field+" value for directory");
	}).always(function() {
		observe.run();
	});
	return val;
  }

};

