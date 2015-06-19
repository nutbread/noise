import re, doc;



# Content creator
class Content(object):
	class Var(object):
		def __init__(self, value=u""):
			self.value = value;

		def set(self, value):
			self.value = value;

		def get(self):
			return self.value;

		def __str__(self):
			return self.value;

	def __init__(self):
		self.value = [];

	def add(self, *values):
		for v in values:
			self.value.append(v);

	def join(self, separator):
		s = u"";
		for i in range(len(self.value)):
			if (i > 0): s += separator;
			s += unicode(self.value[i]);

		return s;

	def get(self):
		return self.join(u"");

	def __str__(self):
		return self.get();



# HTML escaping
__re_html_escaper = re.compile(r"[<>&]");
__re_html_escaper_attr = re.compile(r"[\"'<>&]");
__re_html_escaper_map = {
	u"\"": u"&quot;",
	u"'": u"&apos;",
	u"<": u"&lt;",
	u">": u"&gt;",
	u"&": u"&amp;",
};

def html_escape(value):
	return __re_html_escaper.sub(lambda m: __re_html_escaper_map[m.group(0)], value);
def html_escape_attr(value):
	return __re_html_escaper_attr.sub(lambda m: __re_html_escaper_map[m.group(0)], value);


# Text to id conversion
__re_id_remover = re.compile(r"^[^a-zA-Z0-9$_\+\.]+|[^a-zA-Z0-9$_\+\.]+$|'");
__re_id_replacer = re.compile(r"[^a-zA-Z0-9$_\+\.]+");
def text_to_id(text):
	rep_char = u"-";
	text = __re_id_remover.sub(u"", text);
	text = __re_id_replacer.sub(rep_char, text);

	if (len(text) == 0): return rep_char;
	return text;


# Text formatting
def format_text(text):
	return text;




# Section id
section_id = u"";
ids = {};

def unique_id(id_str, id_list, content):
	# Id
	id = Content.Var(id_str);
	versioned = False;
	if (id.get() in id_list):
		old = id_list[id.get()];
		if (not old[u"versioned"]):
			old[u"versioned"] = True;
			old[u"id"].set(id.get() + u".v1");

		version = 2;
		base = id.get() + ".v{0:d}";
		while (True):
			id.set(base.format(version));
			if (id.get() not in id_list): break;
			version += 1;

		versioned = True;

	id_list[id.get()] = {
		u"id": id,
		u"content": content,
		u"versioned": versioned,
	};

	return id;



# Writer classes
class NodeWriterHeader(doc.NodeWriter):
	def __init__(self, level):
		doc.NodeWriter.__init__(self);
		self.level = level;

	def process(self, node):
		global section_id;

		label = node.get_value_str();
		id = label;
		a = node.get_attribute(u"id");
		if (a is not None and a.value is not None):
			id = a.value;

		section_id = u'{0:s}.'.format(text_to_id(id));

		write(u'<h{0:d} id="{1:s}"><span class="hardlink_text">{2:s}<a class="hardlink" href="#{1:s}"></a></span></h{0:d}>'.format(self.level, html_escape_attr(id), html_escape(label)));

class NodeWriterParagraph(doc.NodeWriter):
	def __init__(self):
		doc.NodeWriter.__init__(self);

	def process(self, node):
		write(u'<p>{0:s}</p>'.format(format_text(node.get_value_str())));

class NodeWriterFunctionList(doc.NodeWriter):
	def __init__(self, is_member):
		doc.NodeWriter.__init__(self);
		self.is_member = is_member;

	def process(self, node):
		if (self.is_member):
			self.__process_member(node);
		else:
			self.__process_function(node);
	def group_start(self, node, pre):
		if (pre is None or not isinstance(pre, self.__class__)):
			write(u'<p><ul class="doc_list">');
	def group_end(self, node, next):
		if (next is None or not isinstance(next, self.__class__)):
			write(u'</ul></p>');

	def __process_function(self, node):
		global section_id, ids;

		content = Content();


		# Basic attributes
		method_name = node.get_value_str();
		template_args = node.get_attributes(u"template");
		return_attr = node.get_attribute(u"return");
		params = node.get_attributes(u"arg");
		obj = node.get_attribute(u"obj");
		obj_template = node.get_attributes(u"obj_template");


		# Id
		function_id = section_id;
		function_id += text_to_id(method_name)
		if (len(template_args) > 0):
			function_id += u'.';
			function_id += u'.'.join([ text_to_id(a.get_value_str()) for a in template_args ]);
		id = unique_id(html_escape_attr(function_id), ids, content);


		# Source
		content.add(
			u'<li><div class="doc_block" id="', id, '">',
			u'<input type="radio" class="doc_block_display_mode doc_block_display_mode_0" value="0" id="', id, u'.display.0" name="', id, u'.display.mode" checked />',
			u'<input type="radio" class="doc_block_display_mode doc_block_display_mode_1" value="1" id="', id, u'.display.1" name="', id, u'.display.mode" />',

			u'<div class="doc_block_indicator hardlink_text"><span class="doc_block_indicator_inner">',
			u'<label class="doc_block_indicator_text" for="', id, u'.display.1"></label>',
			u'<label class="doc_block_indicator_text" for="', id, u'.display.0"></label>',
			u'</span><a class="doc_block_indicator_hardlink hardlink" href="#', id, u'"></a></div>',

			u'<code class="doc_head doc_params_block',
			(u' doc_params_none' if (len(params) == 0) else u''),
			u'">'
		);

		# Type
		if (return_attr is not None):
			content.add(
				u'<span class="doc_return_container"><a class="doc_return" href="#', id, u'.return"><span>',
				html_escape(return_attr.get_value_str()),
				'</span></a> </span>'
			);

		# Object
		if (obj is not None):
			content.add(
				u'<span class="doc_obj">',
				html_escape(obj.get_value_str()),
				u'</span>'
			);
			if (len(obj_template) > 0):
				content.add(u'<span class="doc_template">&lt;');
				for i in range(len(obj_template)):
					if (i > 0): content.add(u', ');
					content.add(
						u'<span class="doc_template_arg">',
						html_escape(obj_template[i].get_value_str()),
						u'</span>'
					);
				content.add(u'&gt;</span>');
			content.add(u'<span class="doc_punct">');
			if (node.get_attribute(u"static") is None):
				content.add(u'.');
			else:
				content.add(u'::');
			content.add(u'</span>');

		# Function name
		content.add(
			u'<label class="doc_name" for="', id, '.display.1"><span>',
			html_escape(method_name),
			u'</span>'
		);

		if (len(template_args) > 0):
			content.add(u'<span class="doc_template">&lt;');
			for i in range(len(template_args)):
				if (i > 0): content.add(u', ');
				content.add(
					u'<span class="doc_template_arg">',
					html_escape(template_args[i].get_value_str()),
					u'</span>'
				);
			content.add(u'&gt;</span>');

		content.add(u'</label><span class="doc_params_outer">(<span class="doc_params">');


		# Params
		for i in range(len(params)):
			# Parameter block
			param = params[i];

			param_classes = u'';
			attr = param.get_attribute(u"keyword")
			if (attr is not None):
				param_classes = u' doc_param_keyword';

			v = param.get_value_str();
			content.add(
				u'<span class="doc_param', param_classes, u'"><span>',
				u'<a class="doc_param_name" href="#', id, u'.', html_escape_attr(text_to_id(v)), u'">'
			);

			attr = param.get_attribute(u"type")
			if (attr is not None):
				content.add(
					u'<span class="doc_param_type">',
					html_escape(attr.get_value_str()),
					u'</span> '
				);

			content.add(
				u'<span>',
				html_escape(v),
				u'</span></a>'
			);

			attr = param.get_attribute(u"default");
			if (attr is not None):
				content.add(
					u'=<span class="doc_param_default">',
					html_escape(attr.get_value_str()),
					u'</span>'
				);

			content.add(u'</span>');
			if (i + 1 < len(params)): content.add(u', ');
			content.add(u'</span>');

		# Params closer
		content.add(u'</span>');
		if (len(params) > 0): content.add(u'<span class="doc_params_placeholder">...</span>');
		content.add(u')</span></code>');


		# Descriptions
		content.add(u'<div class="doc_descriptions">');
		attr = node.get_attribute(u"desc");
		if (attr is not None):
			content.add(
				u'<div class="doc_description doc_description_main">',
				format_text(attr.get_value_str()),
				u'</div>'
			);


		# Param descriptions
		for param in template_args + params:
			# Skip
			if (param.get_attribute(u"desc") is None and param.get_attribute(u"type") is None):
				continue;

			# Write
			v = param.get_value_str();
			content.add(
				u'<div class="doc_description doc_description_param" id="', id, u'.', html_escape_attr(text_to_id(v)), u'"><code>',
				u'<a class="doc_description_param_name" href="#', id, u'.', html_escape_attr(text_to_id(v)), u'"><span>',
				html_escape(v),
				u'</span></a>'
			);

			attr = param.get_attribute(u"type");
			if (attr is not None):
				content.add(
					u'<span class="doc_param_type"> : ',
					html_escape(attr.get_value_str()),
					u'</span>'
				);
			content.add(u'</code>');

			attr = param.get_attribute(u"desc");
			if (attr is not None):
				content.add(
					u'<div class="doc_description_body">',
					format_text(attr.get_value_str()),
					u'</div>'
				);

			content.add(u'</div>');


		# Return description
		if (return_attr is not None):
			content.add(
				u'<div class="doc_description doc_description_return" id="', id, u'.return"><code>',
				u'<a class="doc_description_return_name" href="#', id, u'.return"><span>',
				u'return</span></a>'
			);
			if (return_attr.value is not None):
				content.add(
					u'<span class="doc_param_type"> : ',
					html_escape(return_attr.value),
					u'</span>'
				);
			content.add(u'</code>');


			a = return_attr.get_attribute(u"desc");
			if (a is not None):
				content.add(
					u'<div class="doc_description_body">',
					format_text(a.get_value_str()),
					u'</div>'
				);

			content.add(u'</div>');

		content.add(u'</div>');


		# Close
		content.add(u'</div></li>');

		# Done
		write(content);

	def __process_member(self, node):
		global section_id, ids;

		content = Content();


		# Basic attributes
		node_members = [ node ] + node.get_attributes(u"also");
		template_args = node.get_attributes(u"template");
		obj = node.get_attribute(u"obj");
		obj_template = node.get_attributes(u"obj_template");

		# Id
		function_id = section_id;
		attr = node.get_attribute(u"id");
		function_id += text_to_id((node if (attr is None) else attr).get_value_str())
		if (len(template_args) > 0):
			function_id += u'.';
			function_id += u'.'.join([ text_to_id(a.get_value_str()) for a in template_args ]);
		id = unique_id(html_escape_attr(function_id), ids, content);


		# Id
		a = node.get_attribute(u"id");
		if (a is None):
			main_id = u'{0:s}{1:s}'.format(section_id, text_to_id(node.get_value_str()));
		else:
			main_id = u'{0:s}{1:s}'.format(section_id, text_to_id(a.get_value_str()));
		main_id = html_escape_attr(main_id);


		# Setup
		content.add(
			u'<li><div class="doc_block" id="', id, u'">',
			u'<input type="radio" class="doc_block_display_mode doc_block_display_mode_0" value="0" id="', id, u'.display.0" name="', id, u'.display.mode" checked />',
			u'<input type="radio" class="doc_block_display_mode doc_block_display_mode_1" value="1" id="', id, u'.display.1" name="', id, u'.display.mode" />',

			u'<div class="doc_block_indicator hardlink_text"><span class="doc_block_indicator_inner">',
			u'<label class="doc_block_indicator_text" for="', id, u'.display.1"></label>',
			u'<label class="doc_block_indicator_text" for="', id, u'.display.0"></label>',
			u'</span><a class="doc_block_indicator_hardlink hardlink" href="#', id, u'"></a></div>'
		);


		# Header(s)
		content.add(u'<div class="doc_head">');
		for i in range(len(node_members)):
			member = node_members[i];
			m_id = html_escape_attr(u'{0:s}{1:s}'.format(section_id, text_to_id(member.get_value_str())));

			content.add(
				u'<div class="doc_member_entry" id="',
				m_id,
				u'">'
			);

			# Type
			attr = member.get_attribute(u"type");
			if (attr is not None):
				content.add(
					u'<span class="doc_return_container">',
					html_escape(attr.get_value_str()),
					u'</span> '
				);

			# Object
			if (obj is not None):
				content.add(
					u'<span class="doc_obj">',
					html_escape(obj.get_value_str()),
					u'</span>'
				);
				if (len(obj_template) > 0):
					content.add(u'<span class="doc_template">&lt;');
					for i in range(len(obj_template)):
						if (i > 0): content.add(u', ');
						content.add(
							u'<span class="doc_template_arg">',
							html_escape(obj_template[i].get_value_str()),
							u'</span>'
						);
					content.add(u'&gt;</span>');
				content.add(u'<span class="doc_punct">');
				if (node.get_attribute(u"static") is None):
					content.add(u'.');
				else:
					content.add(u'::');
				content.add(u'</span>');

			content.add(
				u'<span class="doc_member"><span><label class="doc_member_name" for="', id, u'.display.1"><span>',
				html_escape(member.get_value_str()),
				u'</span></label>'
			);

			attr = member.get_attribute(u"value");
			if (attr is not None):
				content.add(
					u'=<span class="doc_member_value">',
					html_escape(attr.get_value_str()),
					u'</span>'
				);

			content.add(u'</span></span>');
			if (i + 1 < len(node_members)): content.add(u',');
			content.add(u'</div>');
		content.add(u'</div>');


		# Description
		attr = node.get_attribute(u"desc");
		if (attr is not None):
			description_id = u'{0:s}.description'.format(main_id);

			content.add(
				u'<div class="doc_descriptions"><div class="doc_description doc_description_main">',
				format_text(attr.get_value_str()),
				u'</div></div>'
			);


		# Complete
		content.add(u'</div></li>');

		# Write
		write(content);



# Document writer
write = None;
def process_doc(filename, write_fn):
	global write;
	write = write_fn;

	f = open(filename, u"rb");
	par = doc.Node.parse(f);
	f.close();

	doc.NodeWriter.process_node(par, descriptor_main);



# Main descriptor
descriptor_main = doc.NodeWriter.Descriptor()
descriptor_main.define(u"h1", NodeWriterHeader, 1);
descriptor_main.define(u"h4", NodeWriterHeader, 4);
descriptor_main.define(u"h6", NodeWriterHeader, 6);
descriptor_main.define(u"p", NodeWriterParagraph);
descriptor_main.define(u"fn", NodeWriterFunctionList, False);
descriptor_main.define(u"member", NodeWriterFunctionList, True);

