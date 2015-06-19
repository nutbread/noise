


(function () {
	"use strict";



	// Module for performing actions as soon as possible
	var on_ready = (function () {

		// Vars
		var callbacks = [],
			check_interval = null,
			check_interval_time = 250;

		// Check if ready and run callbacks
		var callback_check = function () {
			if (
				(document.readyState === "interactive" || document.readyState === "complete") &&
				callbacks !== null
			) {
				// Run callbacks
				var cbs = callbacks,
					cb_count = cbs.length,
					i;

				// Clear
				callbacks = null;

				for (i = 0; i < cb_count; ++i) {
					cbs[i].call(null);
				}

				// Clear events and checking interval
				window.removeEventListener("load", callback_check, false);
				window.removeEventListener("readystatechange", callback_check, false);

				if (check_interval !== null) {
					clearInterval(check_interval);
					check_interval = null;
				}

				// Okay
				return true;
			}

			// Not executed
			return false;
		};

		// Listen
		window.addEventListener("load", callback_check, false);
		window.addEventListener("readystatechange", callback_check, false);

		// Callback adding function
		return function (cb) {
			if (callbacks === null) {
				// Ready to execute
				cb.call(null);
			}
			else {
				// Delay
				callbacks.push(cb);

				// Set a check interval
				if (check_interval === null && callback_check() !== true) {
					check_interval = setInterval(callback_check, check_interval_time);
				}
			}
		};

	})();

	var restyle_noscript = function () {
		// Script
		var nodes = document.querySelectorAll(".script_disabled"),
			i;

		for (i = 0; i < nodes.length; ++i) {
			nodes[i].classList.remove("script_visible");
		}

		nodes = document.querySelectorAll(".script_enabled");
		for (i = 0; i < nodes.length; ++i) {
			nodes[i].classList.add("script_visible");
		}
	};



	// Module to get geometry
	var Geometry = (function () {

		var Rect = function (left, top, right, bottom) {
			this.left = left;
			this.top = top;
			this.right = right;
			this.bottom = bottom;
		};

		var functions = {
			document_rect: function () {
				var win = window,
					doc = document.documentElement,
					left = (win.pageXOffset || doc.scrollLeft || 0) - (doc.clientLeft || 0),
					top = (win.pageYOffset || doc.scrollTop || 0)  - (doc.clientTop || 0);

				return new Rect(
					left,
					top,
					left + (doc.clientWidth || win.innerWidth || 0),
					top + (doc.clientHeight || win.innerHeight || 0)
				);
			},
			object_rect: function (obj) {
				var bounds = obj.getBoundingClientRect(),
					win = window,
					doc = document.documentElement,
					left = (win.pageXOffset || doc.scrollLeft || 0) - (doc.clientLeft || 0),
					top = (win.pageYOffset || doc.scrollTop || 0)  - (doc.clientTop || 0);

				return new Rect(
					left + bounds.left,
					top + bounds.top,
					left + bounds.right,
					top + bounds.bottom
				);
			},
		};

		return functions;

	})();



	// Navigation changing
	var nav = {
		go: function (hash, replace) {
			// Setup url
			var url = window.location.pathname,
				i;

			if (hash !== null) {
				url += hash;
			}

			if (replace) {
				window.history.replaceState({}, "", url);
			}
			else {
				window.history.pushState({}, "", url);
			}
		},
	};



	// Functions
	var on_doc_name_click = function (event) {
		// find doc_block parent
		var block;
		for (block = this.parentNode; block !== null; block = block.parentNode) {
			if (block.classList.contains("doc_block")) {
				// Perform
				var id = block.getAttribute("id"),
					display_mode = block.querySelector(".doc_block_display_mode:checked"),
					display_mode_id = (display_mode ? 1 - (+display_mode.value) : 0),
					target;

				if ((target = block.querySelector(".doc_block_display_mode_" + display_mode_id)) !== null) {
					target.checked = true;
					if (id) {
						if (display_mode_id == 0) {
							if (window.location.hash == "#" + id) {
								nav.go(null, true);
							}
						}
						else {
							nav.go("#" + id, true);
						}
					}
				}

				// Done
				event.preventDefault();
				event.stopPropagation();
				break;
			}
		}
	};
	var on_doc_expand_all_click = function (event) {
		var nodes = document.querySelectorAll(".doc_block_display_mode.doc_block_display_mode_1"),
			i;

		for (i = 0; i < nodes.length; ++i) {
			nodes[i].checked = true;
		}
	};
	var on_doc_shrink_all_click = function (event) {
		var nodes = document.querySelectorAll(".doc_block_display_mode.doc_block_display_mode_0"),
			i;

		for (i = 0; i < nodes.length; ++i) {
			nodes[i].checked = true;
		}
	};



	// Execute
	on_ready(function () {
		// Noscript
		restyle_noscript();

		// Change some label links
		var nodes, i;

		nodes = document.querySelectorAll(".doc_name,.doc_member_name");
		for (i = 0; i < nodes.length; ++i) {
			nodes[i].addEventListener("click", on_doc_name_click, false);
		}

		if ((i = document.getElementById("doc_expand_all"))) {
			i.addEventListener("click", on_doc_expand_all_click, false);
		}

		if ((i = document.getElementById("doc_shrink_all"))) {
			i.addEventListener("click", on_doc_shrink_all_click, false);
		}
	});

})();


