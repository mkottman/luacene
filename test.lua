require 'luacene'

local path = 'index'

print('Creating writer')
local writer = assert(luacene.writer(path))
print('ok', writer)

for i=1,10 do
	writer:addDocument {
		text = 'Hello world',
		uni = 'ľščťžýáíéúäňô',
		num = tostring(i)
	}
end

writer:optimize()
writer:flush()

print('Creating searcher')
local searcher = assert(luacene.searcher(path))
print('ok', searcher)

print('Searching')
local hits = searcher:search('text:world')
assert(hits.length, "hits does not respond to 'length'")
print('ok', hits, hits.length)

local n = hits.length
for i=0,n-1 do
	local d = hits[i]
	print(d)
end
