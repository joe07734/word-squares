#! /usr/bin/env ruby -w

last_word = nil

File.read("/Library/Dictionaries/New Oxford American Dictionary.dict/Contents/dict_body").each_line do |line|

	word_match = line.match(/<o:hw>(.*)<\/o:hw>/)  # isolate the word in the line
	next if !word_match
	
	word = word_match[1].gsub(/<.*?>/m, '')  # remove any xml markup in the word
	next if word.match(/[^a-z]/)  # skip it if it's a proper noun or abbreviation

	next if line.match(/<o:ps> abbreviation <\/o:ps>/)  # skip it if it's a lowercase abbreviation

	next if word == last_word  # skip it if it's the same as the last word (it happens)

	last_word = word
	print(word, "\n")

end
