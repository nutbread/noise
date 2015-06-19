import re;



class Node(object):
	__escape_char = u"\\";
	__newline = u"\n";

	__re_indent = re.compile(r"(\s*)(.*)");
	__re_attribute = re.compile(r"(\s*){0:s}(\w+)(=|\s|$)".format(re.escape(__escape_char)));
	__re_attribute_inner = re.compile(r"(?:^|[^{0:s}]){0:s}(\w+)=\s*".format(re.escape(__escape_char)));
	__re_attribute_inner2 = re.compile(r"{0:s}({0:s}\w+=)".format(re.escape(__escape_char)));

	@classmethod
	def parse(cls, stream):
		# Chain
		parent = cls(None);
		chain_current = ( parent , 0 );
		chain = [ chain_current ];
		newline_count = 0;

		# Read loop
		while (True):
			# Read a line
			line = stream.readline().decode(u"utf-8");
			line_len = len(line);

			# Check end of line
			line_end = None;
			if (line_len > 0 and line[-1] == u"\n"):
				if (line_len > 1 and line[-2] == u"\r"):
					line_end = u"\r\n";
					line = line[0 : -2];
				else:
					line_end = u"\n";
					line = line[0 : -1];

			# Process
			m = cls.__re_attribute.match(line);
			if (m is not None):
				# Indent
				g = m.groups();
				indent_len = cls.__indent_length(g[0]) + 1;

				# Create the node
				node = Node(g[1]);

				# Parse the value
				value = line[m.end() : ].strip();
				if (len(value) > 0 or g[2] == u"="):
					cls.__process_node_value(node, value, False, 0);


				# Move up the chain
				while (indent_len < chain_current[1]):
					# Pop
					chain.pop();
					chain_current = chain[-1];

				# Add to the chain
				chain_next = ( node , indent_len );
				if (indent_len > chain_current[1]):
					# Child
					chain_current[0].attributes.append(node);
					chain.append(chain_next);
				else: # if (indent_len == chain_current[1]):
					# Replace
					chain[-2][0].attributes.append(node);
					chain[-1] = chain_next;
				chain_current = chain_next;
				newline_count = 0;
			else:
				# Remove indentation
				value = line.rstrip();

				# Skip?
				newline_count = cls.__process_node_value(chain_current[0], value, True, newline_count);

			# Done
			if (line_end is None): break;

		# Done
		return parent;
	@classmethod
	def __indent_length(cls, text):
		# Simple length checker
		return len(text);
	@classmethod
	def __indent_remove(cls, text, length):
		# Match indent
		g = cls.__re_indent.match(text).groups();
		return g[0][length : ] + g[1];
	@classmethod
	def __process_node_value(cls, node, text, process_indent, newline_count):
		# Setup
		pos = 0;
		par = node;
		if (len(node.attributes) > 0):
			node = node.attributes[-1];

		# Regex matches
		while (True):
			# Match
			match = cls.__re_attribute_inner.search(text, pos);
			if (match is None):
				value = text[pos : ];
			else:
				value = text[pos : match.start()];

			# Process value
			value = value.rstrip();

			# Process indent
			if (process_indent):
				# Check indentation
				if (node.indent >= 0):
					# Remove indentation
					value = cls.__indent_remove(value, node.indent);
					node_indent = node.indent;
				else:
					# Count indent
					g = cls.__re_indent.match(value).groups();
					value = g[1];
					node_indent = cls.__indent_length(g[0]);

				# Remove escaping
				value_len = len(value);
				g = cls.__re_indent.match(value).groups();
				if (len(g[1]) > 0 and g[1][0] == cls.__escape_char):
					value = g[0] + g[1][1:];
			else:
				# Default values
				value_len = len(value);
				node_indent = node.indent;

			# Replace escaped inner attributes
			v1 = value;
			value = cls.__re_attribute_inner2.sub(u"\\g<1>", value);

			# Apply value
			if (node.value is None):
				# Check length
				if (value_len > 0):
					node.value = value;
					node.indent = node_indent;
			else:
				# Always add
				if (value_len > 0):
					node.value += cls.__newline * (newline_count + 1);
					node.value += value;
					node.indent = node_indent;
					newline_count = 0;
				else:
					newline_count += 1;


			# Next
			if (match is None): break;
			node = Node(match.group(1));
			par.attributes.append(node);
			pos = match.end();
			process_indent = False;

		# Done
		return newline_count;



	def __init__(self, attr_type):
		self.type = attr_type;
		self.value = None;
		self.attributes = [];
		self.indent = -1;

	def get_value_str(self):
		if (self.value is None):
			return u"";
		return self.value;

	def get_attribute(self, attr_type):
		for a in self.attributes:
			if (a.type == attr_type):
				return a;

		return None;

	def get_attributes(self, attr_type):
		attrs = [];
		for a in self.attributes:
			if (a.type == attr_type):
				attrs.append(a);

		return attrs;

	def to_string(self, indent=0):
		s = (u"  " * indent) + u"Node({0:s}".format(self.type);
		if (self.value is not None):
			s += u",{0:s}".format(repr(self.value));
		s += u")";
		for a in self.attributes:
			s += u"\n{0:s}".format(a.to_string(indent + 1));

		return s;



class NodeWriter(object):
	class Descriptor(object):
		def __init__(self):
			self.tags = {};

		def define(self, tag_name, writer, *constructor_args):
			self.tags[tag_name] = ( writer, constructor_args );
			return self;


	def __init__(self):
		pass;

	def process(self, node):
		pass;
	def group_start(self, node, pre):
		pass;
	def group_end(self, node, next):
		pass;

	@classmethod
	def process_node(self, parent, descriptor):
		attr_count = len(parent.attributes);
		if (attr_count == 0):
			return;

		node_pre = None;
		nw_pre = None;
		i = 0;
		while (True):
			node = parent.attributes[i];

			# Skip
			if (node.type not in descriptor.tags):
				i += 1;
				if (i >= attr_count):
					if (nw_pre is not None):
						nw_pre.group_end(node_pre, None);
					break;
				continue;

			d = descriptor.tags[node.type];

			# Change
			if (node_pre is None):
				nw = d[0](*d[1]);
				nw.group_start(node, nw_pre);
			elif (node_pre.type != node.type):
				nw = d[0](*d[1]);
				nw_pre.group_end(node_pre, nw);
				nw.group_start(node, nw_pre);

			# Process
			nw.process(node);

			# Next
			i += 1;
			if (i >= attr_count):
				nw.group_end(node, None);
				break;

			node_pre = node;
			nw_pre = nw;



