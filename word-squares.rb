#! /usr/bin/env ruby -w

#require 'thread'
require 'words_of_length.rb'

$stdout.sync = true
SHUFFLE		 = true
NUM_LETTERS  = (ARGV[0] || "3").to_i

progress = 0
last_progress = 0

Thread.new do
	loop do
		sleep(1)
		print("#{progress}   #{(progress-last_progress)}/sec\n")
		last_progress = progress;
	end
end


words = words_of_length(NUM_LETTERS)

print("Dictionary contains #{words.length} #{NUM_LETTERS}-letter words.\n")

possibilities = 1
NUM_LETTERS.times {possibilities *= words.length}
print("There are #{possibilities} word squares to check.\n")

stems = stems_of_words(words)

if SHUFFLE
	shuffle = shuffle_array(words.length)
else
	shuffle = Array.new(words.length) {|i| i}
end


row_words = []
index = []

row = 1
index[1] = -1

loop do

	progress += 1

	index[row] += 1
	if index[row] == words.length
		row -= 1
		break if row == 0
		next
	end

	row_word = words[shuffle[index[row]]]
	next if row > 1 && row_words[1,row-1].include?(row_word)
	row_words[row] = row_word

	if row < NUM_LETTERS

		ok = true
		NUM_LETTERS.times do |col|

			col_stem = ""
			(1..row).each {|r| col_stem += row_words[r][col,1]}

			if !stems[row].include?(col_stem)
				ok = false
				break
			end

		end
		next if !ok

		row += 1
		index[row] = -1

	else

		ok = true
		col_words = []
		NUM_LETTERS.times do |col|

			col_word = ""
			(1..NUM_LETTERS).each {|r| col_word += row_words[r][col,1]}

			if !words.include?(col_word) || row_words.include?(col_word) || col_words.include?(col_word)
				ok = false
				break
			end
			col_words << col_word

		end
		next if !ok

		print("\n")
		(1..NUM_LETTERS).each {|r| print("#{row_words[r]}   #{index[r]}\n")}
		print("\n")

	end

end
