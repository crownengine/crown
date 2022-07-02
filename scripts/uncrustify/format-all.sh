find src/ -iname '*.h' -o -iname '*.inl' -o -iname '*.cpp' \
	| tr '\n' '\0'                                         \
	| xargs -0 -n1 bash -c                                 \
		'cat $0 | ./scripts/uncrustify/uncrustify-wrapper.sh scripts/uncrustify/cpp.cfg > $0.new && mv $0.new $0'

find tools/ -iname '*.vala' -o -iname '*.c' \
	| tr '\n' '\0'                          \
	| xargs -0 -n1 bash -c                  \
		'cat $0 | ./scripts/uncrustify/uncrustify-wrapper.sh scripts/uncrustify/vala.cfg > $0.new && mv $0.new $0'
