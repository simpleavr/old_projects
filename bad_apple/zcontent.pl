#!/usr/bin/perl -w

#
# ok here is what's needed to generate project content
#  . for content, 
#    we need an avi file for video
#    we need a midi file for audio, prefer about 8 channel simple paino instrument
#  . supporing utility / tools
#    should be done in a linuxish machine or w/ cygwin
#    unixish bin-utils, etc. we need "sort" for sure
#    perl, the internet duct tape to tie things together
#    certian perl modules, like "Image::BMP", "POSIX"
#    midicsv and whatever he requires, this is for midi extraction
#    ffmpeg to extract bitmaps from video
# this is what u get
#  . creates sound.h which contains sound data
#  . creates content.h that contains video data
#

use strict;
use POSIX qw/ceil/;
use Image::BMP;


#________________________________________________________________________________
my $flips = 0;

#________________________________________________________________________________

sub gen_sound {

	# $file .. file to convert
	# $scale.. scale down data required (combine timing)
	# $div.... non-scientific time division, bigger number will play faster
	# $chs.... number of channels wanted

	my ($file, $scale, $div, $chs) = @_;

	# when we are in linux
	# my $midicsv = "midicsv";
	# when we are in windows + cygwin
	my $midicsv = "../../midicsv-1.1/midicsv.exe";

	# i keep my midi files in "../midi/"
	#
	# if things are not what u expected, look at tmp00
	# there are many ways a midi file can be constructed

	system("$midicsv ../midi/$file > tmp00");
	system("cat tmp00 | sort -nk2 > tmp00.sort");

	open I, "< tmp00.sort" or die;

	my @slot = ((0) x $chs);
	my $max = 0;
	my ($note_max, $note_min) = (0, 999);
	my @recs;

	while (<I>) {
		/^(\d+),\s*(\d+),\s*Tempo,\s*(\d+)/i and do {
			my $tempo = $3;
			my $bpm = ceil(60000000 / $tempo);
			#
			# not being used for now, should use to control how
			# fast we play the song, bpm is beat per minute
			# print "tempo..$tempo ($bpm)\n";
		};
		/^(\d+),\s*(\d+),\s*Note_(\w+)_c,\s*(\d+),\s*(\d+),\s*(\d+)/i and do {
			my ($tr, $tm, $on, $ch, $nn, $ve) = ($1, $2, $3, $4, $5, $6);
			$ch > 10  and next;

			#print "($tr, $tm, $on, $ch, $nn, $ve)\n";

			$on eq 'off' and $ve = 0;
			my $good = 0;
			if ($ve) {
				for my $i (0..$chs-1) {
					$slot[$i] or do { 
						$i > $max and $max = $i;
						$slot[$i] = $nn; $good = 1; last; 
					};
				}
			}
			else {
				for my $i (0..$chs-1) {
					$slot[$i] == $nn and do { $slot[$i] = 0; $good = 1; last; };
				}
			}
			$nn > $note_max and $note_max = $nn;
			$nn < $note_min and $note_min = $nn;

			#push @recs, sprintf "%05d: %02d %02d %02d %02d %02d %s", $tm, @slot, ($good ? "": $nn);
			#printf "%06d: %d %02d %02d %02d %02d %02d %s\n", $tm, $ch, @slot, ($good ? "": $nn);
			#push @recs, [$tm/$scale, @slot, ($good ? "--": $nn)];
			push @recs, [$tm/$scale, @slot];
		};
	}

	close I;

	my $cnt = 0;
	my @cur = ();

	unlink "sound.h";
	open O, ">> sound.h" or die;
	print O "const uint8_t sound[] = {\n";

	my @prv = ((0) x ($chs+1));
	for (@recs) {
		@cur && $_->[0] > ($cur[0] + $chs - 1) and do {
			my @to_print;
			my $span = $_->[0] - $cur[0];
			do {
				push @to_print, ($span>0xff ? 0xff : $span);
				$span -= 0xff;
				$span > 0 and do {
					push @to_print, 0x00;
				};
			} while ($span > 0);
			my @notes = ();
			my $bits = 0x00;
			$#prv != $#cur and print "ERR $#prv != $#cur\n";
			for my $i (1..$chs) {
				$bits >>= 1;
				$prv[$i] != $cur[$i] and do {
					$bits |= 1<<($chs-1);
					push @notes, $cur[$i];
				};
			}
			push @to_print, $bits;
			unshift @notes, @to_print;
			while (@notes) {
				printf O " 0x%02x,", shift @notes;
				$cnt++;
				#(++$cnt)%8 or print O "\n";
			}
			print O "\n";
			@prv = @cur;
		};
		@cur = @{$_};
		#print O "(" . (join ',', @cur) . ")\n";
	}
	printf O "\n 0x00, 0x%02x," . " 0x00,"x($chs*2), (1<<$chs)-1;
	$cnt += 2; $cnt += $chs; $cnt += $chs; 

	$max++;
	print O "\n};  // $cnt x $chs bytes, max channels $max, note $note_min, $note_max\n\n";
	print O "#define TICK_DIV	$div\n";
	print O "#define MAX_CH		$chs\n\n";

	close O;

}

sub outb {
	$flips % 16 or print C "\n";
	my $v = shift @_;
	printf C " 0x%02x,", $v;
	print O pack('C', $v);
	$flips++;
}

sub outw {
	$flips % 8 or print C "\n";
	my $v = shift @_;
	printf C " 0x%04x,", $v;
	print O pack('s<', $v);
	$flips++;
}

#________________________________________________________________________________
sub gen_video {
	my $avi = shift @_;
	# this is good for nokia 5110 display, 6 rows of 84 pixels
	# my ($height, $width) = (48, 84);
	# this is good for nokia 7110 display, 8 rows of 96 pixels
	my ($height, $width) = (64, 96);
	#


	-d 'bmp' or system("mkdir bmp");

	# if linux
	# system("ffmpeg -i $avi -f image2 -s 84x48 bmp/foo_%05d.bmp");
	# for windows + cygwin
	system("../../ffmpeg/bin/ffmpeg.exe -i $avi -f image2 -s $width"."x"."$height bmp/foo_%05d.bmp");

	$height /= 8;

	open O, '>:raw', 'out.img' or die "cannot open output binary\n";

	unlink "content.h";
	open C, ">> content.h" or die;

	print C "const uint16_t data[] = {\n";
	my ($cnt, $cur, $run) = (0, 0, 0);
	my ($bcnt, $wcnt) = (0, 0);
	for (`ls bmp/foo_*.bmp`) {
		chomp;
		my $img = new Image::BMP(file => $_,);
		for my $y (0..($height-1)) {
			for my $x (0..($width-1)) {
				my $bits = 0;
				for my $z (0..7) {
					my $dot = $img->xy($x, ($y*8)+$z) ? 0 : 1;
					$bits |= $dot<<$z;
				}
				if (!$bits) {
					$bcnt and do { outw(0x8000 + $bcnt); $bcnt = 0; };
					$wcnt++;
				}
				else {
					if ($bits == 0xff) {
						$wcnt and do { outw(0x4000 + $wcnt); $wcnt = 0; };
						$bcnt++;
					}
					else {
						$bcnt and do { outw(0x8000 + $bcnt); $bcnt = 0; };
						$wcnt and do { outw(0x4000 + $wcnt); $wcnt = 0; };
						outw(0x2000 + $bits);
					}
				}
			}
		}
		$bcnt and do { outw(0x8000 + $bcnt); $bcnt = 0; };
		$wcnt and do { outw(0x4000 + $wcnt); $wcnt = 0; };
		print C "\n// frame $_ done ($flips)\n";
		#++$cnt >= 20 and last;
	}

	outw(0);

	while ($flips%256) { outw(0); }


	print C " \n};  // flips..$flips x 2 bytes\n\n";

	print C "#define FILM_HEIGHT	$height\n";
	print C "#define FILM_WIDTH 	$width\n";
	$flips *= 2;
	$flips /= 64;
	print C "#define SOUND_START 	$flips"."UL	// at this sector (of 64 bytes)\n\n";

	#
	# now read in sound.h and write into sd card image (so that we can run on a 2k g2231)
	#

	-f "sound.h" and do {

		$flips = 0;

		open I, " < sound.h" or die "cannot open sound.h";

		local $/;
		my $full = <I>;

		print C "\n\nconst uint8_t sound[] = {\n";

		for (split /\s*,\s*/, $full) {
			/0x(\w\w)/g and do {
				outb(hex $1);
			};
			/#define\s+.+/ and print C "$_\n";
		}

		while ($flips%512) { outb(0); }

		print C " \n};  // flips..$flips bytes\n\n";
		close I;

	};

	close C;

	close O;
	system("grep define content.h > param.h");

}

#________________________________________________________________________________

gen_sound("bad_apple_piano.mid", 2, 6, 8);
gen_video("../temp/BadApple_30fps.avi");


