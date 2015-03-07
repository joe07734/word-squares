#! /usr/bin/env ruby -w

# return array of all the words from the dictionary that are length x
def words_of_length(x)

	a = []
	File.read("words.txt").each_line do |word|
		word.strip!
		a << word if word.length == x
	end
	return a

end


# return an array of all the word stems (prefixes)
def stems_of_words_of_length(words, x)

	a = []
	words.each do |word|
		a << word[0, x]
	end
	a.uniq!
	return a

end


def stems_of_words(words)

	stems = []
	(1..words[0].length-1).each do |x|
		stems[x] = stems_of_words_of_length(words, x)
	end
	return stems

end


def shuffle_array(length)

	shuffle = Array.new(length) {|i| i}
	shuffle.length.times do |i|
		j = rand(shuffle.length)
		shuffle[i], shuffle[j] = shuffle[j], shuffle[i]
	end
	return shuffle

end
