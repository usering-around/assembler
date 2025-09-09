;this file contains all the errors which are exclusive to the macro expansion step
; invalid macro names
mcro 3invalid_macro
mcroend

mcro big_macro_name_like_very_long_long
mcroend

mcro ihatethe&character
mcroend

; macro is a keyword
mcro add
mcroend

mcro data
mcroend

mcro string
mcroend

mcro extern
mcroend

mcro entry
mcroend

mcro r0
mcroend

mcro r7
mcroend



; mcro without name
mcro 
mcroend

; macro defined as a label
some_label: add r1, r3
mcro some_label
stop
mcroend


; line too big
huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_huge_line_
; just barely enough
this_line_is_really_really_really_really_really_really_really_really_really_long_