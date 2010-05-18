require 'luacene'

local path = 'index'

local stopwords = {'test', 'a'}

print('Creating writer')
local writer = assert(luacene.writer(path, stopwords))
print('ok', writer)

for i=1,10 do
	writer:addDocument {
		text = 'Hello world',
		uni = 'ľščťžýáíéúäňô',
		opt = {
			-- value
			'this is a sample long long long long text long text termvector test',
			-- store option: "yes", "no", "compress"
			'yes',
			-- index option: "no", "tokenized", "untokenized", "nonorms"
			'tokenized',
			-- termvector option: "no", "yes", "positions", "offsets", "positions+offsets"
			'positions+offsets'
		},
		num = tostring(i)
	}
end

writer:optimize()
writer:flush()
writer:close()

print('Creating searcher')
local searcher = assert(luacene.searcher(path, stopwords))
print('ok', searcher)

print('Searching')
local hits = searcher:search('text:world')
assert(hits.length, "hits does not respond to 'length'")
print('ok', hits, hits.length)

local n = hits.length
for i=1,n do
	local d = hits[i]
	print(d, d.text, d.num, d.uni, d.opt)
end

print('Testing stopwords')
local hits2 = searcher:search('opt:a')
assert(hits2.length == 0, "found stopword when it shouldn't")
print('ok', hits2.length)
