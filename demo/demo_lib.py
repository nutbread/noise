import os, subprocess;



# Create noise
def generate_noise(filename, normalize_mode, x, y, width, height, scale, max_radius, power, dimensions, octaves, *extra_args):
	demo_exe = os.path.join(os.path.dirname(os.path.realpath(__file__)), u"demo");

	demo_cmd = [ str(a) for a in [
		demo_exe,
		"-", "binary", normalize_mode,
		x, y,
		width, height,
		scale,
		max_radius,
		power,
		dimensions,
		octaves,
	] + list(extra_args)];

	ffmpeg_cmd = [
		"ffmpeg",
		"-y",
		"-f", "image2pipe",
		"-an",
		"-c:v", "pgm",
		"-i", "pipe:0",
		"-c:v", "png",
		"-compression_level", "100",
		"-pix_fmt", "gray",
		"-f", "image2",
		filename
	];

	# Execute
	p_demo = subprocess.Popen(demo_cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE);
	p_ffmpeg = subprocess.Popen(ffmpeg_cmd, stdin=p_demo.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE);

	# Close streams
	p_demo.stdout.close();
	p_demo.stderr.close();

	# Communicate
	c = p_ffmpeg.communicate()[0];
	p_demo.wait();

	# Done
	return c;


