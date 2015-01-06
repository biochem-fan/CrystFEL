# Fake twinned data

# Usage: fake_twin.rb < input.stream > output.stream

REFLECTION_START_MARKER = "Reflections measured after indexing"
REFLECTION_END_MARKER = "End of reflections"

in_reflist = false
flip = false
while (s = gets)
  s.chomp!
   in_reflist = false if s.start_with?(REFLECTION_END_MARKER)  

  if in_reflist && flip
    elems = s.split
    h = elems[0].to_i
    k = elems[1].to_i
    l = elems[2].to_i

    k, l = l, k # fake operator (h, l, k); no physical meaning

    elems[0] = h.to_s
    elems[1] = k.to_s
    elems[2] = l.to_s
    
    s = elems.join(" ")
  end

  if s.start_with?(REFLECTION_START_MARKER) 
    in_reflist = true 
    flip= (rand(2) == 1)
    s = "flipped = #{flip}\n" + s
  end

  puts s
end
