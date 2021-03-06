/*
	Templated SimplexNoise
*/
#include "SimplexNoise.hpp"



/**
	# The following python script will generate a permutation which will ensure a range of [-1,1] can be reached for the 1D noise:
	import sys, random;
	random.seed(0);

	def validate(a):
		"""
			Makes sure the sequence will result in:
			8 followed by -8
			-8 followed by 8
		"""
		min_ok = False;
		max_ok = False;
		for i in range(len(a)):
			n1 = a[i];
			n2 = a[(i + 1) % len(a)];
			min_ok = min_ok or ((n1 & 0xF) == 0xF and (n2 & 0xF) == 0x7);
			max_ok = max_ok or ((n1 & 0xF) == 0x7 and (n2 & 0xF) == 0xF);
		return min_ok and max_ok;

	def validate2(a):
		"""
			Makes sure the sequence will result in:
			8 followed by 8
			-8 followed by -8
		"""
		min_ok = False;
		max_ok = False;
		for i in range(len(a)):
			n1 = a[i];
			n2 = a[(i + 1) % len(a)];
			min_ok = min_ok or ((n1 & 0xF) == 0xF and (n2 & 0xF) == 0xF);
			max_ok = max_ok or ((n1 & 0xF) == 0x7 and (n2 & 0xF) == 0x7);
		return min_ok and max_ok;

	skip = 0;
	order = list(range(0, 256));
	tries = 0;
	while (True):
		tries += 1;
		random.shuffle(order);
		if (validate(order) and validate2(order)): # the validate2 is not truly necessary
			if (skip <= 0): break;
			skip -= 1;

	sys.stderr.write("Attempts: {0:d}\n".format(tries));

	line_len = 16;
	for i in range(0, len(order), line_len):
		for n in order[i:i+line_len]:
			n = str(n);
			sys.stdout.write("{0: <3s}, ".format(n));
		sys.stdout.write("\n");
*/

const unsigned char SimplexNoiseBase :: perm[512] = {
	18 , 234, 81 , 154, 2  , 206, 228, 251, 195, 75 , 241, 160, 138, 40 , 204, 56 ,
	175, 196, 169, 166, 133, 58 , 161, 101, 254, 155, 174, 145, 231, 134, 71 , 31 ,
	202, 216, 48 , 106, 210, 38 , 110, 177, 200, 94 , 67 , 126, 120, 221, 224, 27 ,
	52 , 89 , 16 , 208, 237, 252, 95 , 218, 74 , 51 , 139, 13 , 129, 98 , 249, 24 ,
	141, 165, 88 , 159, 209, 23 , 125, 70 , 59 , 236, 37 , 151, 163, 179, 123, 142,
	107, 36 , 127, 189, 149, 225, 104, 122, 100, 82 , 140, 147, 6  , 213, 105, 198,
	229, 222, 4  , 116, 132, 153, 108, 109, 12 , 167, 29 , 8  , 150, 233, 253, 144,
	99 , 62 , 10 , 28 , 130, 96 , 205, 248, 184, 17 , 190, 73 , 226, 20 , 245, 157,
	54 , 128, 97 , 146, 3  , 244, 26 , 87 , 193, 255, 168, 112, 143, 119, 240, 188,
	201, 39 , 49 , 53 , 220, 212, 182, 85 , 113, 171, 34 , 79 , 22 , 47 , 238, 217,
	187, 32 , 242, 72 , 43 , 183, 121, 178, 61 , 158, 76 , 181, 84 , 103, 186, 203,
	66 , 83 , 14 , 114, 65 , 33 , 21 , 215, 170, 0  , 93 , 152, 243, 68 , 250, 180,
	111, 235, 118, 11 , 77 , 50 , 55 , 86 , 64 , 136, 124, 92 , 219, 44 , 192, 117,
	197, 148, 211, 80 , 69 , 164, 194, 57 , 1  , 19 , 214, 102, 176, 173, 15 , 63 ,
	9  , 135, 223, 25 , 156, 191, 185, 5  , 45 , 42 , 162, 46 , 227, 90 , 239, 172,
	115, 7  , 35 , 41 , 78 , 199, 247, 137, 91 , 30 , 131, 230, 246, 232, 207, 60 ,

	18 , 234, 81 , 154, 2  , 206, 228, 251, 195, 75 , 241, 160, 138, 40 , 204, 56 ,
	175, 196, 169, 166, 133, 58 , 161, 101, 254, 155, 174, 145, 231, 134, 71 , 31 ,
	202, 216, 48 , 106, 210, 38 , 110, 177, 200, 94 , 67 , 126, 120, 221, 224, 27 ,
	52 , 89 , 16 , 208, 237, 252, 95 , 218, 74 , 51 , 139, 13 , 129, 98 , 249, 24 ,
	141, 165, 88 , 159, 209, 23 , 125, 70 , 59 , 236, 37 , 151, 163, 179, 123, 142,
	107, 36 , 127, 189, 149, 225, 104, 122, 100, 82 , 140, 147, 6  , 213, 105, 198,
	229, 222, 4  , 116, 132, 153, 108, 109, 12 , 167, 29 , 8  , 150, 233, 253, 144,
	99 , 62 , 10 , 28 , 130, 96 , 205, 248, 184, 17 , 190, 73 , 226, 20 , 245, 157,
	54 , 128, 97 , 146, 3  , 244, 26 , 87 , 193, 255, 168, 112, 143, 119, 240, 188,
	201, 39 , 49 , 53 , 220, 212, 182, 85 , 113, 171, 34 , 79 , 22 , 47 , 238, 217,
	187, 32 , 242, 72 , 43 , 183, 121, 178, 61 , 158, 76 , 181, 84 , 103, 186, 203,
	66 , 83 , 14 , 114, 65 , 33 , 21 , 215, 170, 0  , 93 , 152, 243, 68 , 250, 180,
	111, 235, 118, 11 , 77 , 50 , 55 , 86 , 64 , 136, 124, 92 , 219, 44 , 192, 117,
	197, 148, 211, 80 , 69 , 164, 194, 57 , 1  , 19 , 214, 102, 176, 173, 15 , 63 ,
	9  , 135, 223, 25 , 156, 191, 185, 5  , 45 , 42 , 162, 46 , 227, 90 , 239, 172,
	115, 7  , 35 , 41 , 78 , 199, 247, 137, 91 , 30 , 131, 230, 246, 232, 207, 60 ,

/*	151, 160, 137, 91 , 90 , 15 , 131, 13 , 201, 95 , 96 , 53 , 194, 233, 7  , 225,
	140, 36 , 103, 30 , 69 , 142, 8  , 99 , 37 , 240, 21 , 10 , 23 , 190, 6  , 148,
	247, 120, 234, 75 , 0  , 26 , 197, 62 , 94 , 252, 219, 203, 117, 35 , 11 , 32 ,
	57 , 177, 33 , 88 , 237, 149, 56 , 87 , 174, 20 , 125, 136, 171, 168, 68 , 175,
	74 , 165, 71 , 134, 139, 48 , 27 , 166, 77 , 146, 158, 231, 83 , 111, 229, 122,
	60 , 211, 133, 230, 220, 105, 92 , 41 , 55 , 46 , 245, 40 , 244, 102, 143, 54 ,
	65 , 25 , 63 , 161, 1  , 216, 80 , 73 , 209, 76 , 132, 187, 208, 89 , 18 , 169,
	200, 196, 135, 130, 116, 188, 159, 86 , 164, 100, 109, 198, 173, 186, 3  , 64 ,
	52 , 217, 226, 250, 124, 123, 5  , 202, 38 , 147, 118, 126, 255, 82 , 85 , 212,
	207, 206, 59 , 227, 47 , 16 , 58 , 17 , 182, 189, 28 , 42 , 223, 183, 170, 213,
	119, 248, 152, 2  , 44 , 154, 163, 70 , 221, 153, 101, 155, 167, 43 , 172, 9  ,
	129, 22 , 39 , 253, 19 , 98 , 108, 110, 79 , 113, 224, 232, 178, 185, 112, 104,
	218, 246, 97 , 228, 251, 34 , 242, 193, 238, 210, 144, 12 , 191, 179, 162, 241,
	81 , 51 , 145, 235, 249, 14 , 239, 107, 49 , 192, 214, 31 , 181, 199, 106, 157,
	184, 84 , 204, 176, 115, 121, 50 , 45 , 127, 4  , 150, 254, 138, 236, 205, 93 ,
	222, 114, 67 , 29 , 24 , 72 , 243, 141, 128, 195, 78 , 66 , 215, 61 , 156, 180,

	151, 160, 137, 91 , 90 , 15 , 131, 13 , 201, 95 , 96 , 53 , 194, 233, 7  , 225,
	140, 36 , 103, 30 , 69 , 142, 8  , 99 , 37 , 240, 21 , 10 , 23 , 190, 6  , 148,
	247, 120, 234, 75 , 0  , 26 , 197, 62 , 94 , 252, 219, 203, 117, 35 , 11 , 32 ,
	57 , 177, 33 , 88 , 237, 149, 56 , 87 , 174, 20 , 125, 136, 171, 168, 68 , 175,
	74 , 165, 71 , 134, 139, 48 , 27 , 166, 77 , 146, 158, 231, 83 , 111, 229, 122,
	60 , 211, 133, 230, 220, 105, 92 , 41 , 55 , 46 , 245, 40 , 244, 102, 143, 54 ,
	65 , 25 , 63 , 161, 1  , 216, 80 , 73 , 209, 76 , 132, 187, 208, 89 , 18 , 169,
	200, 196, 135, 130, 116, 188, 159, 86 , 164, 100, 109, 198, 173, 186, 3  , 64 ,
	52 , 217, 226, 250, 124, 123, 5  , 202, 38 , 147, 118, 126, 255, 82 , 85 , 212,
	207, 206, 59 , 227, 47 , 16 , 58 , 17 , 182, 189, 28 , 42 , 223, 183, 170, 213,
	119, 248, 152, 2  , 44 , 154, 163, 70 , 221, 153, 101, 155, 167, 43 , 172, 9  ,
	129, 22 , 39 , 253, 19 , 98 , 108, 110, 79 , 113, 224, 232, 178, 185, 112, 104,
	218, 246, 97 , 228, 251, 34 , 242, 193, 238, 210, 144, 12 , 191, 179, 162, 241,
	81 , 51 , 145, 235, 249, 14 , 239, 107, 49 , 192, 214, 31 , 181, 199, 106, 157,
	184, 84 , 204, 176, 115, 121, 50 , 45 , 127, 4  , 150, 254, 138, 236, 205, 93 ,
	222, 114, 67 , 29 , 24 , 72 , 243, 141, 128, 195, 78 , 66 , 215, 61 , 156, 180,*/
};

