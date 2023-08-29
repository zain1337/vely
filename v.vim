" Language: Vely
" Vim syntax file
" Maintainer: Sergio Mijatovic
" Latest Revision: May 23 2022
so $VIMRUNTIME/syntax/c.vim
syntax sync minlines=10000
highlight velyConstruct ctermfg=6
highlight velyClause ctermfg=5
hi def link vv_h_other    Normal
syn region vv_r_inline_dot start="<<[[:space:]]*[\.]" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_dot '<<[[:space:]]*[\.]' contained containedin=vv_r_inline_dot
    syn match vv_h_print_inline_dot '>>' contained containedin=vv_r_inline_dot
    hi def link vv_h_print_inline_dot    velyConstruct
syn region vv_r_excl start="^[[:space:]]*[!]" skip="\\[[:space:]]*$" end="$"  keepend
syn match vv_h_excl_begin '^[[:space:]]*[!]' contained containedin=vv_r_excl
hi def link vv_h_excl_begin    velyConstruct
syn match vv_h_dot '^[[:space:]]*\.'
syn region vv_r_at start="^[[:space:]]*[@]" skip="\\[[:space:]]*$" end="$"  keepend
syn match vv_h_at_begin '^[[:space:]]*[@]' contained containedin=vv_r_at
hi def link vv_h_at_begin    velyConstruct
hi def link vv_h_dot    velyConstruct
syn region vv_r_construct_end_read_line start="^[[:space:]]*end-read-line" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_end_read_line,vv_r_inline_end_read_line,vv_r_at
    syn match vv_h_construct_end_read_line "^[[:space:]]*end-read-line" contained containedin=vv_r_construct_end_read_line
    syn match vv_h_clause_end_read_line "[=]\@<=define \@=" contained containedin=vv_r_construct_end_read_line
    syn match vv_h_clause_end_read_line " define \@=" contained containedin=vv_r_construct_end_read_line
    hi def link vv_h_clause_end_read_line    velyClause
    hi def link vv_h_construct_end_read_line    velyConstruct
    hi def link vv_h_print_inline_end_read_line    velyConstruct
syn region vv_r_construct_read_line start="^[[:space:]]*read-line" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_read_line,vv_r_inline_read_line,vv_r_at
    syn match vv_h_construct_read_line "^[[:space:]]*read-line" contained containedin=vv_r_construct_read_line
    syn match vv_h_clause_read_line " delimiter \@=" contained containedin=vv_r_construct_read_line
    syn match vv_h_clause_read_line " status \@=" contained containedin=vv_r_construct_read_line
    syn match vv_h_clause_read_line " to \@=" contained containedin=vv_r_construct_read_line
    syn match vv_h_clause_read_line "[=]\@<=define \@=" contained containedin=vv_r_construct_read_line
    syn match vv_h_clause_read_line " define \@=" contained containedin=vv_r_construct_read_line
    hi def link vv_h_clause_read_line    velyClause
    hi def link vv_h_construct_read_line    velyConstruct
    hi def link vv_h_print_inline_read_line    velyConstruct
syn region vv_r_construct_double_left_par start="^[[:space:]]*((" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_double_left_par,vv_r_inline_double_left_par,vv_r_at
    syn match vv_h_construct_double_left_par "^[[:space:]]*((" contained containedin=vv_r_construct_double_left_par
    syn match vv_h_clause_double_left_par "[=]\@<=define \@=" contained containedin=vv_r_construct_double_left_par
    syn match vv_h_clause_double_left_par " define \@=" contained containedin=vv_r_construct_double_left_par
    hi def link vv_h_clause_double_left_par    velyClause
    hi def link vv_h_construct_double_left_par    velyConstruct
    hi def link vv_h_print_inline_double_left_par    velyConstruct
syn region vv_r_construct_write_string start="^[[:space:]]*write-string" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_write_string,vv_r_inline_write_string,vv_r_at
    syn match vv_h_construct_write_string "^[[:space:]]*write-string" contained containedin=vv_r_construct_write_string
    syn match vv_h_clause_write_string "[=]\@<=define \@=" contained containedin=vv_r_construct_write_string
    syn match vv_h_clause_write_string " define \@=" contained containedin=vv_r_construct_write_string
    syn match vv_h_clause_write_string "[=]\@<=define \@=" contained containedin=vv_r_construct_write_string
    syn match vv_h_clause_write_string " define \@=" contained containedin=vv_r_construct_write_string
    hi def link vv_h_clause_write_string    velyClause
    hi def link vv_h_construct_write_string    velyConstruct
    hi def link vv_h_print_inline_write_string    velyConstruct
syn region vv_r_construct_double_right_par start="^[[:space:]]*))" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_double_right_par,vv_r_inline_double_right_par,vv_r_at
    syn match vv_h_construct_double_right_par "^[[:space:]]*))" contained containedin=vv_r_construct_double_right_par
    syn match vv_h_clause_double_right_par " bytes-written \@=" contained containedin=vv_r_construct_double_right_par
    syn match vv_h_clause_double_right_par " notrim \@=" contained containedin=vv_r_construct_double_right_par
    syn match vv_h_clause_double_right_par " notrim$" contained containedin=vv_r_construct_double_right_par
    syn match vv_h_clause_double_right_par "[=]\@<=define \@=" contained containedin=vv_r_construct_double_right_par
    syn match vv_h_clause_double_right_par " define \@=" contained containedin=vv_r_construct_double_right_par
    hi def link vv_h_clause_double_right_par    velyClause
    hi def link vv_h_construct_double_right_par    velyConstruct
    hi def link vv_h_print_inline_double_right_par    velyConstruct
syn region vv_r_construct_end_write_string start="^[[:space:]]*end-write-string" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_end_write_string,vv_r_inline_end_write_string,vv_r_at
    syn match vv_h_construct_end_write_string "^[[:space:]]*end-write-string" contained containedin=vv_r_construct_end_write_string
    syn match vv_h_clause_end_write_string " bytes-written \@=" contained containedin=vv_r_construct_end_write_string
    syn match vv_h_clause_end_write_string " notrim \@=" contained containedin=vv_r_construct_end_write_string
    syn match vv_h_clause_end_write_string " notrim$" contained containedin=vv_r_construct_end_write_string
    syn match vv_h_clause_end_write_string "[=]\@<=define \@=" contained containedin=vv_r_construct_end_write_string
    syn match vv_h_clause_end_write_string " define \@=" contained containedin=vv_r_construct_end_write_string
    syn match vv_h_clause_end_write_string "[=]\@<=define \@=" contained containedin=vv_r_construct_end_write_string
    syn match vv_h_clause_end_write_string " define \@=" contained containedin=vv_r_construct_end_write_string
    hi def link vv_h_clause_end_write_string    velyClause
    hi def link vv_h_construct_end_write_string    velyConstruct
    hi def link vv_h_print_inline_end_write_string    velyConstruct
syn region vv_r_construct_open_file start="^[[:space:]]*open-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_open_file,vv_r_inline_open_file,vv_r_at
    syn match vv_h_construct_open_file "^[[:space:]]*open-file" contained containedin=vv_r_construct_open_file
    syn match vv_h_clause_open_file " file-id \@=" contained containedin=vv_r_construct_open_file
    syn match vv_h_clause_open_file " new-truncate \@=" contained containedin=vv_r_construct_open_file
    syn match vv_h_clause_open_file " new-truncate$" contained containedin=vv_r_construct_open_file
    syn match vv_h_clause_open_file " status \@=" contained containedin=vv_r_construct_open_file
    syn match vv_h_clause_open_file "[=]\@<=define \@=" contained containedin=vv_r_construct_open_file
    syn match vv_h_clause_open_file " define \@=" contained containedin=vv_r_construct_open_file
    hi def link vv_h_clause_open_file    velyClause
    hi def link vv_h_construct_open_file    velyConstruct
    hi def link vv_h_print_inline_open_file    velyConstruct
syn region vv_r_construct_close_file start="^[[:space:]]*close-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_close_file,vv_r_inline_close_file,vv_r_at
    syn match vv_h_construct_close_file "^[[:space:]]*close-file" contained containedin=vv_r_construct_close_file
    syn match vv_h_clause_close_file " file-id \@=" contained containedin=vv_r_construct_close_file
    syn match vv_h_clause_close_file " status \@=" contained containedin=vv_r_construct_close_file
    syn match vv_h_clause_close_file "[=]\@<=define \@=" contained containedin=vv_r_construct_close_file
    syn match vv_h_clause_close_file " define \@=" contained containedin=vv_r_construct_close_file
    hi def link vv_h_clause_close_file    velyClause
    hi def link vv_h_construct_close_file    velyConstruct
    hi def link vv_h_print_inline_close_file    velyConstruct
syn region vv_r_construct_file_position start="^[[:space:]]*file-position" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_file_position,vv_r_inline_file_position,vv_r_at
    syn match vv_h_construct_file_position "^[[:space:]]*file-position" contained containedin=vv_r_construct_file_position
    syn match vv_h_clause_file_position " file-id \@=" contained containedin=vv_r_construct_file_position
    syn match vv_h_clause_file_position " get \@=" contained containedin=vv_r_construct_file_position
    syn match vv_h_clause_file_position " set \@=" contained containedin=vv_r_construct_file_position
    syn match vv_h_clause_file_position " status \@=" contained containedin=vv_r_construct_file_position
    syn match vv_h_clause_file_position "[=]\@<=define \@=" contained containedin=vv_r_construct_file_position
    syn match vv_h_clause_file_position " define \@=" contained containedin=vv_r_construct_file_position
    hi def link vv_h_clause_file_position    velyClause
    hi def link vv_h_construct_file_position    velyConstruct
    hi def link vv_h_print_inline_file_position    velyConstruct
syn region vv_r_construct_exit_request start="^[[:space:]]*exit-request" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_exit_request,vv_r_inline_exit_request,vv_r_at
    syn match vv_h_construct_exit_request "^[[:space:]]*exit-request" contained containedin=vv_r_construct_exit_request
    syn match vv_h_clause_exit_request "[=]\@<=define \@=" contained containedin=vv_r_construct_exit_request
    syn match vv_h_clause_exit_request " define \@=" contained containedin=vv_r_construct_exit_request
    hi def link vv_h_clause_exit_request    velyClause
    hi def link vv_h_construct_exit_request    velyConstruct
    hi def link vv_h_print_inline_exit_request    velyConstruct
syn region vv_r_construct_finish_output start="^[[:space:]]*finish-output" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_finish_output,vv_r_inline_finish_output,vv_r_at
    syn match vv_h_construct_finish_output "^[[:space:]]*finish-output" contained containedin=vv_r_construct_finish_output
    syn match vv_h_clause_finish_output "[=]\@<=define \@=" contained containedin=vv_r_construct_finish_output
    syn match vv_h_clause_finish_output " define \@=" contained containedin=vv_r_construct_finish_output
    hi def link vv_h_clause_finish_output    velyClause
    hi def link vv_h_construct_finish_output    velyConstruct
    hi def link vv_h_print_inline_finish_output    velyConstruct
syn region vv_r_construct_copy_file start="^[[:space:]]*copy-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_copy_file,vv_r_inline_copy_file,vv_r_at
    syn match vv_h_construct_copy_file "^[[:space:]]*copy-file" contained containedin=vv_r_construct_copy_file
    syn match vv_h_clause_copy_file " status \@=" contained containedin=vv_r_construct_copy_file
    syn match vv_h_clause_copy_file " to \@=" contained containedin=vv_r_construct_copy_file
    syn match vv_h_clause_copy_file "[=]\@<=define \@=" contained containedin=vv_r_construct_copy_file
    syn match vv_h_clause_copy_file " define \@=" contained containedin=vv_r_construct_copy_file
    hi def link vv_h_clause_copy_file    velyClause
    hi def link vv_h_construct_copy_file    velyConstruct
    hi def link vv_h_print_inline_copy_file    velyConstruct
syn region vv_r_construct_resize_mem start="^[[:space:]]*resize-mem" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_resize_mem,vv_r_inline_resize_mem,vv_r_at
    syn match vv_h_construct_resize_mem "^[[:space:]]*resize-mem" contained containedin=vv_r_construct_resize_mem
    syn match vv_h_clause_resize_mem " size \@=" contained containedin=vv_r_construct_resize_mem
    syn match vv_h_clause_resize_mem "[=]\@<=define \@=" contained containedin=vv_r_construct_resize_mem
    syn match vv_h_clause_resize_mem " define \@=" contained containedin=vv_r_construct_resize_mem
    hi def link vv_h_clause_resize_mem    velyClause
    hi def link vv_h_construct_resize_mem    velyConstruct
    hi def link vv_h_print_inline_resize_mem    velyConstruct
syn region vv_r_construct_new_mem start="^[[:space:]]*new-mem" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_new_mem,vv_r_inline_new_mem,vv_r_at
    syn match vv_h_construct_new_mem "^[[:space:]]*new-mem" contained containedin=vv_r_construct_new_mem
    syn match vv_h_clause_new_mem " block-size \@=" contained containedin=vv_r_construct_new_mem
    syn match vv_h_clause_new_mem " init \@=" contained containedin=vv_r_construct_new_mem
    syn match vv_h_clause_new_mem " init$" contained containedin=vv_r_construct_new_mem
    syn match vv_h_clause_new_mem " size \@=" contained containedin=vv_r_construct_new_mem
    syn match vv_h_clause_new_mem "[=]\@<=define \@=" contained containedin=vv_r_construct_new_mem
    syn match vv_h_clause_new_mem " define \@=" contained containedin=vv_r_construct_new_mem
    hi def link vv_h_clause_new_mem    velyClause
    hi def link vv_h_construct_new_mem    velyConstruct
    hi def link vv_h_print_inline_new_mem    velyConstruct
syn region vv_r_construct_delete_mem start="^[[:space:]]*delete-mem" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_delete_mem,vv_r_inline_delete_mem,vv_r_at
    syn match vv_h_construct_delete_mem "^[[:space:]]*delete-mem" contained containedin=vv_r_construct_delete_mem
    syn match vv_h_clause_delete_mem " status \@=" contained containedin=vv_r_construct_delete_mem
    syn match vv_h_clause_delete_mem "[=]\@<=define \@=" contained containedin=vv_r_construct_delete_mem
    syn match vv_h_clause_delete_mem " define \@=" contained containedin=vv_r_construct_delete_mem
    hi def link vv_h_clause_delete_mem    velyClause
    hi def link vv_h_construct_delete_mem    velyConstruct
    hi def link vv_h_print_inline_delete_mem    velyConstruct
syn region vv_r_construct_manage_memory start="^[[:space:]]*manage-memory" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_manage_memory,vv_r_inline_manage_memory,vv_r_at
    syn match vv_h_construct_manage_memory "^[[:space:]]*manage-memory" contained containedin=vv_r_construct_manage_memory
    syn match vv_h_clause_manage_memory " off \@=" contained containedin=vv_r_construct_manage_memory
    syn match vv_h_clause_manage_memory " off$" contained containedin=vv_r_construct_manage_memory
    syn match vv_h_clause_manage_memory " on \@=" contained containedin=vv_r_construct_manage_memory
    syn match vv_h_clause_manage_memory " on$" contained containedin=vv_r_construct_manage_memory
    syn match vv_h_clause_manage_memory "[=]\@<=define \@=" contained containedin=vv_r_construct_manage_memory
    syn match vv_h_clause_manage_memory " define \@=" contained containedin=vv_r_construct_manage_memory
    hi def link vv_h_clause_manage_memory    velyClause
    hi def link vv_h_construct_manage_memory    velyConstruct
    hi def link vv_h_print_inline_manage_memory    velyConstruct
syn region vv_r_construct_read_hash start="^[[:space:]]*read-hash" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_read_hash,vv_r_inline_read_hash,vv_r_at
    syn match vv_h_construct_read_hash "^[[:space:]]*read-hash" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " begin \@=" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " begin$" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " delete \@=" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " delete$" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " key \@=" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " old-key \@=" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " status \@=" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " traverse \@=" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " traverse$" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " value \@=" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash "[=]\@<=define \@=" contained containedin=vv_r_construct_read_hash
    syn match vv_h_clause_read_hash " define \@=" contained containedin=vv_r_construct_read_hash
    hi def link vv_h_clause_read_hash    velyClause
    hi def link vv_h_construct_read_hash    velyConstruct
    hi def link vv_h_print_inline_read_hash    velyConstruct
syn region vv_r_construct_write_hash start="^[[:space:]]*write-hash" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_write_hash,vv_r_inline_write_hash,vv_r_at
    syn match vv_h_construct_write_hash "^[[:space:]]*write-hash" contained containedin=vv_r_construct_write_hash
    syn match vv_h_clause_write_hash " key \@=" contained containedin=vv_r_construct_write_hash
    syn match vv_h_clause_write_hash " old-key \@=" contained containedin=vv_r_construct_write_hash
    syn match vv_h_clause_write_hash " old-value \@=" contained containedin=vv_r_construct_write_hash
    syn match vv_h_clause_write_hash " status \@=" contained containedin=vv_r_construct_write_hash
    syn match vv_h_clause_write_hash " value \@=" contained containedin=vv_r_construct_write_hash
    syn match vv_h_clause_write_hash "[=]\@<=define \@=" contained containedin=vv_r_construct_write_hash
    syn match vv_h_clause_write_hash " define \@=" contained containedin=vv_r_construct_write_hash
    hi def link vv_h_clause_write_hash    velyClause
    hi def link vv_h_construct_write_hash    velyConstruct
    hi def link vv_h_print_inline_write_hash    velyConstruct
syn region vv_r_construct_new_hash start="^[[:space:]]*new-hash" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_new_hash,vv_r_inline_new_hash,vv_r_at
    syn match vv_h_construct_new_hash "^[[:space:]]*new-hash" contained containedin=vv_r_construct_new_hash
    syn match vv_h_clause_new_hash " size \@=" contained containedin=vv_r_construct_new_hash
    syn match vv_h_clause_new_hash "[=]\@<=define \@=" contained containedin=vv_r_construct_new_hash
    syn match vv_h_clause_new_hash " define \@=" contained containedin=vv_r_construct_new_hash
    hi def link vv_h_clause_new_hash    velyClause
    hi def link vv_h_construct_new_hash    velyConstruct
    hi def link vv_h_print_inline_new_hash    velyConstruct
syn region vv_r_construct_resize_hash start="^[[:space:]]*resize-hash" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_resize_hash,vv_r_inline_resize_hash,vv_r_at
    syn match vv_h_construct_resize_hash "^[[:space:]]*resize-hash" contained containedin=vv_r_construct_resize_hash
    syn match vv_h_clause_resize_hash " size \@=" contained containedin=vv_r_construct_resize_hash
    syn match vv_h_clause_resize_hash "[=]\@<=define \@=" contained containedin=vv_r_construct_resize_hash
    syn match vv_h_clause_resize_hash " define \@=" contained containedin=vv_r_construct_resize_hash
    hi def link vv_h_clause_resize_hash    velyClause
    hi def link vv_h_construct_resize_hash    velyConstruct
    hi def link vv_h_print_inline_resize_hash    velyConstruct
syn region vv_r_construct_purge_hash start="^[[:space:]]*purge-hash" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_purge_hash,vv_r_inline_purge_hash,vv_r_at
    syn match vv_h_construct_purge_hash "^[[:space:]]*purge-hash" contained containedin=vv_r_construct_purge_hash
    syn match vv_h_clause_purge_hash " delete \@=" contained containedin=vv_r_construct_purge_hash
    syn match vv_h_clause_purge_hash " delete$" contained containedin=vv_r_construct_purge_hash
    syn match vv_h_clause_purge_hash "[=]\@<=define \@=" contained containedin=vv_r_construct_purge_hash
    syn match vv_h_clause_purge_hash " define \@=" contained containedin=vv_r_construct_purge_hash
    hi def link vv_h_clause_purge_hash    velyClause
    hi def link vv_h_construct_purge_hash    velyConstruct
    hi def link vv_h_print_inline_purge_hash    velyConstruct
syn region vv_r_construct_get_hash start="^[[:space:]]*get-hash" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_get_hash,vv_r_inline_get_hash,vv_r_at
    syn match vv_h_construct_get_hash "^[[:space:]]*get-hash" contained containedin=vv_r_construct_get_hash
    syn match vv_h_clause_get_hash " average-reads \@=" contained containedin=vv_r_construct_get_hash
    syn match vv_h_clause_get_hash " length \@=" contained containedin=vv_r_construct_get_hash
    syn match vv_h_clause_get_hash " size \@=" contained containedin=vv_r_construct_get_hash
    syn match vv_h_clause_get_hash "[=]\@<=define \@=" contained containedin=vv_r_construct_get_hash
    syn match vv_h_clause_get_hash " define \@=" contained containedin=vv_r_construct_get_hash
    hi def link vv_h_clause_get_hash    velyClause
    hi def link vv_h_construct_get_hash    velyConstruct
    hi def link vv_h_print_inline_get_hash    velyConstruct
syn region vv_r_construct_read_file start="^[[:space:]]*read-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_read_file,vv_r_inline_read_file,vv_r_at
    syn match vv_h_construct_read_file "^[[:space:]]*read-file" contained containedin=vv_r_construct_read_file
    syn match vv_h_clause_read_file " file-id \@=" contained containedin=vv_r_construct_read_file
    syn match vv_h_clause_read_file " length \@=" contained containedin=vv_r_construct_read_file
    syn match vv_h_clause_read_file " position \@=" contained containedin=vv_r_construct_read_file
    syn match vv_h_clause_read_file " status \@=" contained containedin=vv_r_construct_read_file
    syn match vv_h_clause_read_file " to \@=" contained containedin=vv_r_construct_read_file
    syn match vv_h_clause_read_file "[=]\@<=define \@=" contained containedin=vv_r_construct_read_file
    syn match vv_h_clause_read_file " define \@=" contained containedin=vv_r_construct_read_file
    hi def link vv_h_clause_read_file    velyClause
    hi def link vv_h_construct_read_file    velyConstruct
    hi def link vv_h_print_inline_read_file    velyConstruct
syn region vv_r_construct_write_file start="^[[:space:]]*write-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_write_file,vv_r_inline_write_file,vv_r_at
    syn match vv_h_construct_write_file "^[[:space:]]*write-file" contained containedin=vv_r_construct_write_file
    syn match vv_h_clause_write_file " append \@=" contained containedin=vv_r_construct_write_file
    syn match vv_h_clause_write_file " append$" contained containedin=vv_r_construct_write_file
    syn match vv_h_clause_write_file " file-id \@=" contained containedin=vv_r_construct_write_file
    syn match vv_h_clause_write_file " from \@=" contained containedin=vv_r_construct_write_file
    syn match vv_h_clause_write_file " length \@=" contained containedin=vv_r_construct_write_file
    syn match vv_h_clause_write_file " position \@=" contained containedin=vv_r_construct_write_file
    syn match vv_h_clause_write_file " status \@=" contained containedin=vv_r_construct_write_file
    syn match vv_h_clause_write_file "[=]\@<=define \@=" contained containedin=vv_r_construct_write_file
    syn match vv_h_clause_write_file " define \@=" contained containedin=vv_r_construct_write_file
    hi def link vv_h_clause_write_file    velyClause
    hi def link vv_h_construct_write_file    velyConstruct
    hi def link vv_h_print_inline_write_file    velyConstruct
syn region vv_r_construct_read_fifo start="^[[:space:]]*read-fifo" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_read_fifo,vv_r_inline_read_fifo,vv_r_at
    syn match vv_h_construct_read_fifo "^[[:space:]]*read-fifo" contained containedin=vv_r_construct_read_fifo
    syn match vv_h_clause_read_fifo " key \@=" contained containedin=vv_r_construct_read_fifo
    syn match vv_h_clause_read_fifo " value \@=" contained containedin=vv_r_construct_read_fifo
    syn match vv_h_clause_read_fifo "[=]\@<=define \@=" contained containedin=vv_r_construct_read_fifo
    syn match vv_h_clause_read_fifo " define \@=" contained containedin=vv_r_construct_read_fifo
    hi def link vv_h_clause_read_fifo    velyClause
    hi def link vv_h_construct_read_fifo    velyConstruct
    hi def link vv_h_print_inline_read_fifo    velyConstruct
syn region vv_r_construct_purge_fifo start="^[[:space:]]*purge-fifo" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_purge_fifo,vv_r_inline_purge_fifo,vv_r_at
    syn match vv_h_construct_purge_fifo "^[[:space:]]*purge-fifo" contained containedin=vv_r_construct_purge_fifo
    syn match vv_h_clause_purge_fifo " delete \@=" contained containedin=vv_r_construct_purge_fifo
    syn match vv_h_clause_purge_fifo " delete$" contained containedin=vv_r_construct_purge_fifo
    syn match vv_h_clause_purge_fifo "[=]\@<=define \@=" contained containedin=vv_r_construct_purge_fifo
    syn match vv_h_clause_purge_fifo " define \@=" contained containedin=vv_r_construct_purge_fifo
    hi def link vv_h_clause_purge_fifo    velyClause
    hi def link vv_h_construct_purge_fifo    velyConstruct
    hi def link vv_h_print_inline_purge_fifo    velyConstruct
syn region vv_r_construct_rewind_fifo start="^[[:space:]]*rewind-fifo" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_rewind_fifo,vv_r_inline_rewind_fifo,vv_r_at
    syn match vv_h_construct_rewind_fifo "^[[:space:]]*rewind-fifo" contained containedin=vv_r_construct_rewind_fifo
    syn match vv_h_clause_rewind_fifo "[=]\@<=define \@=" contained containedin=vv_r_construct_rewind_fifo
    syn match vv_h_clause_rewind_fifo " define \@=" contained containedin=vv_r_construct_rewind_fifo
    hi def link vv_h_clause_rewind_fifo    velyClause
    hi def link vv_h_construct_rewind_fifo    velyConstruct
    hi def link vv_h_print_inline_rewind_fifo    velyConstruct
syn region vv_r_construct_write_fifo start="^[[:space:]]*write-fifo" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_write_fifo,vv_r_inline_write_fifo,vv_r_at
    syn match vv_h_construct_write_fifo "^[[:space:]]*write-fifo" contained containedin=vv_r_construct_write_fifo
    syn match vv_h_clause_write_fifo " key \@=" contained containedin=vv_r_construct_write_fifo
    syn match vv_h_clause_write_fifo " value \@=" contained containedin=vv_r_construct_write_fifo
    syn match vv_h_clause_write_fifo "[=]\@<=define \@=" contained containedin=vv_r_construct_write_fifo
    syn match vv_h_clause_write_fifo " define \@=" contained containedin=vv_r_construct_write_fifo
    hi def link vv_h_clause_write_fifo    velyClause
    hi def link vv_h_construct_write_fifo    velyConstruct
    hi def link vv_h_print_inline_write_fifo    velyConstruct
syn region vv_r_construct_new_fifo start="^[[:space:]]*new-fifo" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_new_fifo,vv_r_inline_new_fifo,vv_r_at
    syn match vv_h_construct_new_fifo "^[[:space:]]*new-fifo" contained containedin=vv_r_construct_new_fifo
    syn match vv_h_clause_new_fifo "[=]\@<=define \@=" contained containedin=vv_r_construct_new_fifo
    syn match vv_h_clause_new_fifo " define \@=" contained containedin=vv_r_construct_new_fifo
    hi def link vv_h_clause_new_fifo    velyClause
    hi def link vv_h_construct_new_fifo    velyConstruct
    hi def link vv_h_print_inline_new_fifo    velyConstruct
syn region vv_r_construct_unused_var start="^[[:space:]]*unused-var" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_unused_var,vv_r_inline_unused_var,vv_r_at
    syn match vv_h_construct_unused_var "^[[:space:]]*unused-var" contained containedin=vv_r_construct_unused_var
    syn match vv_h_clause_unused_var "[=]\@<=define \@=" contained containedin=vv_r_construct_unused_var
    syn match vv_h_clause_unused_var " define \@=" contained containedin=vv_r_construct_unused_var
    hi def link vv_h_clause_unused_var    velyClause
    hi def link vv_h_construct_unused_var    velyConstruct
    hi def link vv_h_print_inline_unused_var    velyConstruct
syn region vv_r_construct_split_string start="^[[:space:]]*split-string" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_split_string,vv_r_inline_split_string,vv_r_at
    syn match vv_h_construct_split_string "^[[:space:]]*split-string" contained containedin=vv_r_construct_split_string
    syn match vv_h_clause_split_string " delete \@=" contained containedin=vv_r_construct_split_string
    syn match vv_h_clause_split_string " to \@=" contained containedin=vv_r_construct_split_string
    syn match vv_h_clause_split_string " with \@=" contained containedin=vv_r_construct_split_string
    syn match vv_h_clause_split_string "[=]\@<=define \@=" contained containedin=vv_r_construct_split_string
    syn match vv_h_clause_split_string " define \@=" contained containedin=vv_r_construct_split_string
    hi def link vv_h_clause_split_string    velyClause
    hi def link vv_h_construct_split_string    velyConstruct
    hi def link vv_h_print_inline_split_string    velyConstruct
syn region vv_r_construct_task_param start="^[[:space:]]*task-param" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_task_param,vv_r_inline_task_param,vv_r_at
    syn match vv_h_construct_task_param "^[[:space:]]*task-param" contained containedin=vv_r_construct_task_param
    syn match vv_h_clause_task_param "[=]\@<=define \@=" contained containedin=vv_r_construct_task_param
    syn match vv_h_clause_task_param " define \@=" contained containedin=vv_r_construct_task_param
    hi def link vv_h_clause_task_param    velyClause
    hi def link vv_h_construct_task_param    velyConstruct
    hi def link vv_h_print_inline_task_param    velyConstruct
syn region vv_r_construct_input_param start="^[[:space:]]*input-param" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_input_param,vv_r_inline_input_param,vv_r_at
    syn match vv_h_construct_input_param "^[[:space:]]*input-param" contained containedin=vv_r_construct_input_param
    syn match vv_h_clause_input_param "[=]\@<=define \@=" contained containedin=vv_r_construct_input_param
    syn match vv_h_clause_input_param " define \@=" contained containedin=vv_r_construct_input_param
    syn match vv_h_clause_input_param "[=]\@<=define \@=" contained containedin=vv_r_construct_input_param
    syn match vv_h_clause_input_param " define \@=" contained containedin=vv_r_construct_input_param
    hi def link vv_h_clause_input_param    velyClause
    hi def link vv_h_construct_input_param    velyConstruct
    hi def link vv_h_print_inline_input_param    velyConstruct
syn region vv_r_construct_end_task start="^[[:space:]]*end-task" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_end_task,vv_r_inline_end_task,vv_r_at
    syn match vv_h_construct_end_task "^[[:space:]]*end-task" contained containedin=vv_r_construct_end_task
    syn match vv_h_clause_end_task "[=]\@<=define \@=" contained containedin=vv_r_construct_end_task
    syn match vv_h_clause_end_task " define \@=" contained containedin=vv_r_construct_end_task
    hi def link vv_h_clause_end_task    velyClause
    hi def link vv_h_construct_end_task    velyConstruct
    hi def link vv_h_print_inline_end_task    velyConstruct
syn region vv_r_construct_else_task start="^[[:space:]]*else-task" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_else_task,vv_r_inline_else_task,vv_r_at
    syn match vv_h_construct_else_task "^[[:space:]]*else-task" contained containedin=vv_r_construct_else_task
    syn match vv_h_clause_else_task " other \@=" contained containedin=vv_r_construct_else_task
    syn match vv_h_clause_else_task " other$" contained containedin=vv_r_construct_else_task
    syn match vv_h_clause_else_task "[=]\@<=define \@=" contained containedin=vv_r_construct_else_task
    syn match vv_h_clause_else_task " define \@=" contained containedin=vv_r_construct_else_task
    hi def link vv_h_clause_else_task    velyClause
    hi def link vv_h_construct_else_task    velyConstruct
    hi def link vv_h_print_inline_else_task    velyConstruct
syn region vv_r_construct_if_task start="^[[:space:]]*if-task" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_if_task,vv_r_inline_if_task,vv_r_at
    syn match vv_h_construct_if_task "^[[:space:]]*if-task" contained containedin=vv_r_construct_if_task
    syn match vv_h_clause_if_task " other \@=" contained containedin=vv_r_construct_if_task
    syn match vv_h_clause_if_task " other$" contained containedin=vv_r_construct_if_task
    syn match vv_h_clause_if_task "[=]\@<=define \@=" contained containedin=vv_r_construct_if_task
    syn match vv_h_clause_if_task " define \@=" contained containedin=vv_r_construct_if_task
    syn match vv_h_clause_if_task "[=]\@<=define \@=" contained containedin=vv_r_construct_if_task
    syn match vv_h_clause_if_task " define \@=" contained containedin=vv_r_construct_if_task
    hi def link vv_h_clause_if_task    velyClause
    hi def link vv_h_construct_if_task    velyConstruct
    hi def link vv_h_print_inline_if_task    velyConstruct
syn region vv_r_construct_set_input start="^[[:space:]]*set-input" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_set_input,vv_r_inline_set_input,vv_r_at
    syn match vv_h_construct_set_input "^[[:space:]]*set-input" contained containedin=vv_r_construct_set_input
    syn match vv_h_clause_set_input "[=]\@<=define \@=" contained containedin=vv_r_construct_set_input
    syn match vv_h_clause_set_input " define \@=" contained containedin=vv_r_construct_set_input
    hi def link vv_h_clause_set_input    velyClause
    hi def link vv_h_construct_set_input    velyConstruct
    hi def link vv_h_print_inline_set_input    velyConstruct
syn region vv_r_construct_request_body start="^[[:space:]]*request-body" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_request_body,vv_r_inline_request_body,vv_r_at
    syn match vv_h_construct_request_body "^[[:space:]]*request-body" contained containedin=vv_r_construct_request_body
    syn match vv_h_clause_request_body " length \@=" contained containedin=vv_r_construct_request_body
    syn match vv_h_clause_request_body "[=]\@<=define \@=" contained containedin=vv_r_construct_request_body
    syn match vv_h_clause_request_body " define \@=" contained containedin=vv_r_construct_request_body
    hi def link vv_h_clause_request_body    velyClause
    hi def link vv_h_construct_request_body    velyConstruct
    hi def link vv_h_print_inline_request_body    velyConstruct
syn region vv_r_construct_delete_cookie start="^[[:space:]]*delete-cookie" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_delete_cookie,vv_r_inline_delete_cookie,vv_r_at
    syn match vv_h_construct_delete_cookie "^[[:space:]]*delete-cookie" contained containedin=vv_r_construct_delete_cookie
    syn match vv_h_clause_delete_cookie " path \@=" contained containedin=vv_r_construct_delete_cookie
    syn match vv_h_clause_delete_cookie " secure \@=" contained containedin=vv_r_construct_delete_cookie
    syn match vv_h_clause_delete_cookie " secure$" contained containedin=vv_r_construct_delete_cookie
    syn match vv_h_clause_delete_cookie " status \@=" contained containedin=vv_r_construct_delete_cookie
    syn match vv_h_clause_delete_cookie "[=]\@<=define \@=" contained containedin=vv_r_construct_delete_cookie
    syn match vv_h_clause_delete_cookie " define \@=" contained containedin=vv_r_construct_delete_cookie
    hi def link vv_h_clause_delete_cookie    velyClause
    hi def link vv_h_construct_delete_cookie    velyConstruct
    hi def link vv_h_print_inline_delete_cookie    velyConstruct
syn region vv_r_construct_get_cookie start="^[[:space:]]*get-cookie" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_get_cookie,vv_r_inline_get_cookie,vv_r_at
    syn match vv_h_construct_get_cookie "^[[:space:]]*get-cookie" contained containedin=vv_r_construct_get_cookie
    syn match vv_h_clause_get_cookie "[=]\@<=define \@=" contained containedin=vv_r_construct_get_cookie
    syn match vv_h_clause_get_cookie " define \@=" contained containedin=vv_r_construct_get_cookie
    hi def link vv_h_clause_get_cookie    velyClause
    hi def link vv_h_construct_get_cookie    velyConstruct
    hi def link vv_h_print_inline_get_cookie    velyConstruct
syn region vv_r_construct_set_cookie start="^[[:space:]]*set-cookie" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_set_cookie,vv_r_inline_set_cookie,vv_r_at
    syn match vv_h_construct_set_cookie "^[[:space:]]*set-cookie" contained containedin=vv_r_construct_set_cookie
    syn match vv_h_clause_set_cookie " expires \@=" contained containedin=vv_r_construct_set_cookie
    syn match vv_h_clause_set_cookie " no-http-only \@=" contained containedin=vv_r_construct_set_cookie
    syn match vv_h_clause_set_cookie " no-http-only$" contained containedin=vv_r_construct_set_cookie
    syn match vv_h_clause_set_cookie " path \@=" contained containedin=vv_r_construct_set_cookie
    syn match vv_h_clause_set_cookie " same-site \@=" contained containedin=vv_r_construct_set_cookie
    syn match vv_h_clause_set_cookie " secure \@=" contained containedin=vv_r_construct_set_cookie
    syn match vv_h_clause_set_cookie " secure$" contained containedin=vv_r_construct_set_cookie
    syn match vv_h_clause_set_cookie "[=]\@<=define \@=" contained containedin=vv_r_construct_set_cookie
    syn match vv_h_clause_set_cookie " define \@=" contained containedin=vv_r_construct_set_cookie
    hi def link vv_h_clause_set_cookie    velyClause
    hi def link vv_h_construct_set_cookie    velyConstruct
    hi def link vv_h_print_inline_set_cookie    velyConstruct
syn region vv_r_construct_report_error start="^[[:space:]]*report-error" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_report_error,vv_r_inline_report_error,vv_r_at
    syn match vv_h_construct_report_error "^[[:space:]]*report-error" contained containedin=vv_r_construct_report_error
    syn match vv_h_clause_report_error "[=]\@<=define \@=" contained containedin=vv_r_construct_report_error
    syn match vv_h_clause_report_error " define \@=" contained containedin=vv_r_construct_report_error
    hi def link vv_h_clause_report_error    velyClause
    hi def link vv_h_construct_report_error    velyConstruct
    hi def link vv_h_print_inline_report_error    velyConstruct
syn region vv_r_construct_copy_string start="^[[:space:]]*copy-string" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_copy_string,vv_r_inline_copy_string,vv_r_at
    syn match vv_h_construct_copy_string "^[[:space:]]*copy-string" contained containedin=vv_r_construct_copy_string
    syn match vv_h_clause_copy_string " bytes-written \@=" contained containedin=vv_r_construct_copy_string
    syn match vv_h_clause_copy_string " length \@=" contained containedin=vv_r_construct_copy_string
    syn match vv_h_clause_copy_string " to \@=" contained containedin=vv_r_construct_copy_string
    syn match vv_h_clause_copy_string "[=]\@<=define \@=" contained containedin=vv_r_construct_copy_string
    syn match vv_h_clause_copy_string " define \@=" contained containedin=vv_r_construct_copy_string
    hi def link vv_h_clause_copy_string    velyClause
    hi def link vv_h_construct_copy_string    velyConstruct
    hi def link vv_h_print_inline_copy_string    velyConstruct
syn region vv_r_construct_trace_run start="^[[:space:]]*trace-run" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_trace_run,vv_r_inline_trace_run,vv_r_at
    syn match vv_h_construct_trace_run "^[[:space:]]*trace-run" contained containedin=vv_r_construct_trace_run
    syn match vv_h_clause_trace_run "[=]\@<=define \@=" contained containedin=vv_r_construct_trace_run
    syn match vv_h_clause_trace_run " define \@=" contained containedin=vv_r_construct_trace_run
    hi def link vv_h_clause_trace_run    velyClause
    hi def link vv_h_construct_trace_run    velyConstruct
    hi def link vv_h_print_inline_trace_run    velyConstruct
syn region vv_r_construct_pf_url start="^[[:space:]]*pf-url" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_pf_url,vv_r_inline_pf_url,vv_r_at
    syn match vv_h_construct_pf_url "^[[:space:]]*pf-url" contained containedin=vv_r_construct_pf_url
    syn match vv_h_clause_pf_url " bytes-written \@=" contained containedin=vv_r_construct_pf_url
    syn match vv_h_print_inline_pf_url " bytes-written \@=" contained containedin=vv_r_inline_pf_url
    syn match vv_h_print_inline_pf_url " define \@=" contained containedin=vv_r_inline_pf_url
    syn match vv_h_clause_pf_url " format \@=" contained containedin=vv_r_construct_pf_url
    syn match vv_h_print_inline_pf_url " format \@=" contained containedin=vv_r_inline_pf_url
    syn match vv_h_print_inline_pf_url " define \@=" contained containedin=vv_r_inline_pf_url
    syn match vv_h_clause_pf_url " to \@=" contained containedin=vv_r_construct_pf_url
    syn match vv_h_print_inline_pf_url " to \@=" contained containedin=vv_r_inline_pf_url
    syn match vv_h_print_inline_pf_url " define \@=" contained containedin=vv_r_inline_pf_url
    syn match vv_h_clause_pf_url " to-error \@=" contained containedin=vv_r_construct_pf_url
    syn match vv_h_clause_pf_url " to-error$" contained containedin=vv_r_construct_pf_url
    syn match vv_h_print_inline_pf_url " to-error\(>>\)\@=" contained containedin=vv_r_inline_pf_url
    syn match vv_h_print_inline_pf_url " to-error \@=" contained containedin=vv_r_inline_pf_url
    syn match vv_h_clause_pf_url "[=]\@<=define \@=" contained containedin=vv_r_construct_pf_url
    syn match vv_h_clause_pf_url " define \@=" contained containedin=vv_r_construct_pf_url
    syn match vv_h_print_inline_pf_url " define \@=" contained containedin=vv_r_inline_pf_url
    syn match vv_h_print_inline_pf_url " define \@=" contained containedin=vv_r_inline_pf_url
    syn region vv_r_inline_pf_url start="<<[[:space:]]*pf-url \@=" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_pf_url '<<[[:space:]]*pf-url \@=' contained containedin=vv_r_inline_pf_url
    syn match vv_h_print_inline_pf_url '>>' contained containedin=vv_r_inline_pf_url
    hi def link vv_h_clause_pf_url    velyClause
    hi def link vv_h_construct_pf_url    velyConstruct
    hi def link vv_h_print_inline_pf_url    velyConstruct
syn region vv_r_construct_pf_web start="^[[:space:]]*pf-web" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_pf_web,vv_r_inline_pf_web,vv_r_at
    syn match vv_h_construct_pf_web "^[[:space:]]*pf-web" contained containedin=vv_r_construct_pf_web
    syn match vv_h_clause_pf_web " bytes-written \@=" contained containedin=vv_r_construct_pf_web
    syn match vv_h_print_inline_pf_web " bytes-written \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_print_inline_pf_web " define \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_clause_pf_web " format \@=" contained containedin=vv_r_construct_pf_web
    syn match vv_h_print_inline_pf_web " format \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_print_inline_pf_web " define \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_clause_pf_web " to \@=" contained containedin=vv_r_construct_pf_web
    syn match vv_h_print_inline_pf_web " to \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_print_inline_pf_web " define \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_clause_pf_web " to-error \@=" contained containedin=vv_r_construct_pf_web
    syn match vv_h_clause_pf_web " to-error$" contained containedin=vv_r_construct_pf_web
    syn match vv_h_print_inline_pf_web " to-error\(>>\)\@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_print_inline_pf_web " to-error \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_clause_pf_web "[=]\@<=define \@=" contained containedin=vv_r_construct_pf_web
    syn match vv_h_clause_pf_web " define \@=" contained containedin=vv_r_construct_pf_web
    syn match vv_h_print_inline_pf_web " define \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_print_inline_pf_web " define \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_clause_pf_web "[=]\@<=define \@=" contained containedin=vv_r_construct_pf_web
    syn match vv_h_clause_pf_web " define \@=" contained containedin=vv_r_construct_pf_web
    syn match vv_h_print_inline_pf_web " define \@=" contained containedin=vv_r_inline_pf_web
    syn match vv_h_print_inline_pf_web " define \@=" contained containedin=vv_r_inline_pf_web
    syn region vv_r_inline_pf_web start="<<[[:space:]]*pf-web \@=" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_pf_web '<<[[:space:]]*pf-web \@=' contained containedin=vv_r_inline_pf_web
    syn match vv_h_print_inline_pf_web '>>' contained containedin=vv_r_inline_pf_web
    hi def link vv_h_clause_pf_web    velyClause
    hi def link vv_h_construct_pf_web    velyConstruct
    hi def link vv_h_print_inline_pf_web    velyConstruct
syn region vv_r_construct_pf_out start="^[[:space:]]*pf-out" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_pf_out,vv_r_inline_pf_out,vv_r_at
    syn match vv_h_construct_pf_out "^[[:space:]]*pf-out" contained containedin=vv_r_construct_pf_out
    syn match vv_h_clause_pf_out " bytes-written \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_print_inline_pf_out " bytes-written \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_print_inline_pf_out " define \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_clause_pf_out " format \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_print_inline_pf_out " format \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_print_inline_pf_out " define \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_clause_pf_out " to \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_print_inline_pf_out " to \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_print_inline_pf_out " define \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_clause_pf_out " to-error \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_clause_pf_out " to-error$" contained containedin=vv_r_construct_pf_out
    syn match vv_h_print_inline_pf_out " to-error\(>>\)\@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_print_inline_pf_out " to-error \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_clause_pf_out "[=]\@<=define \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_clause_pf_out " define \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_print_inline_pf_out " define \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_print_inline_pf_out " define \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_clause_pf_out "[=]\@<=define \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_clause_pf_out " define \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_print_inline_pf_out " define \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_print_inline_pf_out " define \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_clause_pf_out "[=]\@<=define \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_clause_pf_out " define \@=" contained containedin=vv_r_construct_pf_out
    syn match vv_h_print_inline_pf_out " define \@=" contained containedin=vv_r_inline_pf_out
    syn match vv_h_print_inline_pf_out " define \@=" contained containedin=vv_r_inline_pf_out
    syn region vv_r_inline_pf_out start="<<[[:space:]]*pf-out \@=" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_pf_out '<<[[:space:]]*pf-out \@=' contained containedin=vv_r_inline_pf_out
    syn match vv_h_print_inline_pf_out '>>' contained containedin=vv_r_inline_pf_out
    hi def link vv_h_clause_pf_out    velyClause
    hi def link vv_h_construct_pf_out    velyConstruct
    hi def link vv_h_print_inline_pf_out    velyConstruct
syn region vv_r_construct_set_req start="^[[:space:]]*set-req" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_set_req,vv_r_inline_set_req,vv_r_at
    syn match vv_h_construct_set_req "^[[:space:]]*set-req" contained containedin=vv_r_construct_set_req
    syn match vv_h_clause_set_req " data \@=" contained containedin=vv_r_construct_set_req
    syn match vv_h_clause_set_req "[=]\@<=define \@=" contained containedin=vv_r_construct_set_req
    syn match vv_h_clause_set_req " define \@=" contained containedin=vv_r_construct_set_req
    hi def link vv_h_clause_set_req    velyClause
    hi def link vv_h_construct_set_req    velyConstruct
    hi def link vv_h_print_inline_set_req    velyConstruct
syn region vv_r_construct_set_app start="^[[:space:]]*set-app" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_set_app,vv_r_inline_set_app,vv_r_at
    syn match vv_h_construct_set_app "^[[:space:]]*set-app" contained containedin=vv_r_construct_set_app
    syn match vv_h_clause_set_app " process-data \@=" contained containedin=vv_r_construct_set_app
    syn match vv_h_clause_set_app "[=]\@<=define \@=" contained containedin=vv_r_construct_set_app
    syn match vv_h_clause_set_app " define \@=" contained containedin=vv_r_construct_set_app
    hi def link vv_h_clause_set_app    velyClause
    hi def link vv_h_construct_set_app    velyConstruct
    hi def link vv_h_print_inline_set_app    velyConstruct
syn region vv_r_construct_flush_output start="^[[:space:]]*flush-output" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_flush_output,vv_r_inline_flush_output,vv_r_at
    syn match vv_h_construct_flush_output "^[[:space:]]*flush-output" contained containedin=vv_r_construct_flush_output
    syn match vv_h_clause_flush_output "[=]\@<=define \@=" contained containedin=vv_r_construct_flush_output
    syn match vv_h_clause_flush_output " define \@=" contained containedin=vv_r_construct_flush_output
    hi def link vv_h_clause_flush_output    velyClause
    hi def link vv_h_construct_flush_output    velyConstruct
    hi def link vv_h_print_inline_flush_output    velyConstruct
syn region vv_r_construct_get_req start="^[[:space:]]*get-req" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_get_req,vv_r_inline_get_req,vv_r_at
    syn match vv_h_construct_get_req "^[[:space:]]*get-req" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " arg-count \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " arg-count$" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " arg-value \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " content-type \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " cookie \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " cookie-count \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " cookie-count$" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " data \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " errno \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " errno$" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " error \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " header \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " input-count \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " input-count$" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " input-name \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " input-value \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " method \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " name \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " name$" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " process-id \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " process-id$" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " referring-url \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " referring-url$" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " to \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " trace-file \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " trace-file$" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req "[=]\@<=define \@=" contained containedin=vv_r_construct_get_req
    syn match vv_h_clause_get_req " define \@=" contained containedin=vv_r_construct_get_req
    hi def link vv_h_clause_get_req    velyClause
    hi def link vv_h_construct_get_req    velyConstruct
    hi def link vv_h_print_inline_get_req    velyConstruct
syn region vv_r_construct_get_app start="^[[:space:]]*get-app" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_get_app,vv_r_inline_get_app,vv_r_at
    syn match vv_h_construct_get_app "^[[:space:]]*get-app" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " db-vendor \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " directory \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " directory$" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " file-directory \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " file-directory$" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " name \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " name$" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " path \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " process-data \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " to \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " trace-directory \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " trace-directory$" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " upload-size \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " upload-size$" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app "[=]\@<=define \@=" contained containedin=vv_r_construct_get_app
    syn match vv_h_clause_get_app " define \@=" contained containedin=vv_r_construct_get_app
    hi def link vv_h_clause_get_app    velyClause
    hi def link vv_h_construct_get_app    velyConstruct
    hi def link vv_h_print_inline_get_app    velyConstruct
syn region vv_r_construct_get_sys start="^[[:space:]]*get-sys" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_get_sys,vv_r_inline_get_sys,vv_r_at
    syn match vv_h_construct_get_sys "^[[:space:]]*get-sys" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " directory \@=" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " directory$" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " environment \@=" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " os-name \@=" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " os-name$" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " os-version \@=" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " os-version$" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " to \@=" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " web-environment \@=" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys "[=]\@<=define \@=" contained containedin=vv_r_construct_get_sys
    syn match vv_h_clause_get_sys " define \@=" contained containedin=vv_r_construct_get_sys
    hi def link vv_h_clause_get_sys    velyClause
    hi def link vv_h_construct_get_sys    velyConstruct
    hi def link vv_h_print_inline_get_sys    velyConstruct
syn region vv_r_construct_match_regex start="^[[:space:]]*match-regex" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_match_regex,vv_r_inline_match_regex,vv_r_at
    syn match vv_h_construct_match_regex "^[[:space:]]*match-regex" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " cache \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " cache$" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " case-insensitive \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " case-insensitive$" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " clear-cache \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " in \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " replace-with \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " result \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " result-length \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " single-match \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " single-match$" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " status \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " utf8 \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " utf8$" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex "[=]\@<=define \@=" contained containedin=vv_r_construct_match_regex
    syn match vv_h_clause_match_regex " define \@=" contained containedin=vv_r_construct_match_regex
    hi def link vv_h_clause_match_regex    velyClause
    hi def link vv_h_construct_match_regex    velyConstruct
    hi def link vv_h_print_inline_match_regex    velyConstruct
syn region vv_r_construct_get_time start="^[[:space:]]*get-time" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_get_time,vv_r_inline_get_time,vv_r_at
    syn match vv_h_construct_get_time "^[[:space:]]*get-time" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " day \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " format \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " hour \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " minute \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " month \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " second \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " timezone \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " to \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " year \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time "[=]\@<=define \@=" contained containedin=vv_r_construct_get_time
    syn match vv_h_clause_get_time " define \@=" contained containedin=vv_r_construct_get_time
    hi def link vv_h_clause_get_time    velyClause
    hi def link vv_h_construct_get_time    velyConstruct
    hi def link vv_h_print_inline_get_time    velyConstruct
syn region vv_r_construct_uniq_file start="^[[:space:]]*uniq-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_uniq_file,vv_r_inline_uniq_file,vv_r_at
    syn match vv_h_construct_uniq_file "^[[:space:]]*uniq-file" contained containedin=vv_r_construct_uniq_file
    syn match vv_h_clause_uniq_file " temporary \@=" contained containedin=vv_r_construct_uniq_file
    syn match vv_h_clause_uniq_file " temporary$" contained containedin=vv_r_construct_uniq_file
    syn match vv_h_clause_uniq_file "[=]\@<=define \@=" contained containedin=vv_r_construct_uniq_file
    syn match vv_h_clause_uniq_file " define \@=" contained containedin=vv_r_construct_uniq_file
    hi def link vv_h_clause_uniq_file    velyClause
    hi def link vv_h_construct_uniq_file    velyConstruct
    hi def link vv_h_print_inline_uniq_file    velyConstruct
syn region vv_r_construct_call_server start="^[[:space:]]*call-server" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_call_server,vv_r_inline_call_server,vv_r_at
    syn match vv_h_construct_call_server "^[[:space:]]*call-server" contained containedin=vv_r_construct_call_server
    syn match vv_h_clause_call_server " array-count \@=" contained containedin=vv_r_construct_call_server
    syn match vv_h_clause_call_server " finished-okay \@=" contained containedin=vv_r_construct_call_server
    syn match vv_h_clause_call_server " started \@=" contained containedin=vv_r_construct_call_server
    syn match vv_h_clause_call_server " status \@=" contained containedin=vv_r_construct_call_server
    syn match vv_h_clause_call_server "[=]\@<=define \@=" contained containedin=vv_r_construct_call_server
    syn match vv_h_clause_call_server " define \@=" contained containedin=vv_r_construct_call_server
    hi def link vv_h_clause_call_server    velyClause
    hi def link vv_h_construct_call_server    velyConstruct
    hi def link vv_h_print_inline_call_server    velyConstruct
syn region vv_r_construct_delete_server start="^[[:space:]]*delete-server" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_delete_server,vv_r_inline_delete_server,vv_r_at
    syn match vv_h_construct_delete_server "^[[:space:]]*delete-server" contained containedin=vv_r_construct_delete_server
    syn match vv_h_clause_delete_server "[=]\@<=define \@=" contained containedin=vv_r_construct_delete_server
    syn match vv_h_clause_delete_server " define \@=" contained containedin=vv_r_construct_delete_server
    hi def link vv_h_clause_delete_server    velyClause
    hi def link vv_h_construct_delete_server    velyConstruct
    hi def link vv_h_print_inline_delete_server    velyConstruct
syn region vv_r_construct_read_server start="^[[:space:]]*read-server" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_read_server,vv_r_inline_read_server,vv_r_at
    syn match vv_h_construct_read_server "^[[:space:]]*read-server" contained containedin=vv_r_construct_read_server
    syn match vv_h_clause_read_server " data \@=" contained containedin=vv_r_construct_read_server
    syn match vv_h_clause_read_server " data-length \@=" contained containedin=vv_r_construct_read_server
    syn match vv_h_clause_read_server " error \@=" contained containedin=vv_r_construct_read_server
    syn match vv_h_clause_read_server " error-length \@=" contained containedin=vv_r_construct_read_server
    syn match vv_h_clause_read_server " request-status \@=" contained containedin=vv_r_construct_read_server
    syn match vv_h_clause_read_server " status \@=" contained containedin=vv_r_construct_read_server
    syn match vv_h_clause_read_server " status-text \@=" contained containedin=vv_r_construct_read_server
    syn match vv_h_clause_read_server "[=]\@<=define \@=" contained containedin=vv_r_construct_read_server
    syn match vv_h_clause_read_server " define \@=" contained containedin=vv_r_construct_read_server
    hi def link vv_h_clause_read_server    velyClause
    hi def link vv_h_construct_read_server    velyConstruct
    hi def link vv_h_print_inline_read_server    velyConstruct
syn region vv_r_construct_new_server start="^[[:space:]]*new-server" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_new_server,vv_r_inline_new_server,vv_r_at
    syn match vv_h_construct_new_server "^[[:space:]]*new-server" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " app-path \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " content \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " content-length \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " content-type \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " environment \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " location \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " method \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " request-body \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " request-path \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " timeout \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " url-payload \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server "[=]\@<=define \@=" contained containedin=vv_r_construct_new_server
    syn match vv_h_clause_new_server " define \@=" contained containedin=vv_r_construct_new_server
    hi def link vv_h_clause_new_server    velyClause
    hi def link vv_h_construct_new_server    velyConstruct
    hi def link vv_h_print_inline_new_server    velyConstruct
syn region vv_r_construct_call_web start="^[[:space:]]*call-web" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_call_web,vv_r_inline_call_web,vv_r_at
    syn match vv_h_construct_call_web "^[[:space:]]*call-web" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " cert \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " content \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " content-length \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " content-type \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " cookie-jar \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " custom \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " error \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " fields \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " files \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " method \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " no-cert \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " no-cert$" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " request-body \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " request-headers \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " response \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " response-code \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " response-headers \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " status \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " timeout \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web "[=]\@<=define \@=" contained containedin=vv_r_construct_call_web
    syn match vv_h_clause_call_web " define \@=" contained containedin=vv_r_construct_call_web
    hi def link vv_h_clause_call_web    velyClause
    hi def link vv_h_construct_call_web    velyConstruct
    hi def link vv_h_print_inline_call_web    velyConstruct
syn region vv_r_construct_delete_file start="^[[:space:]]*delete-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_delete_file,vv_r_inline_delete_file,vv_r_at
    syn match vv_h_construct_delete_file "^[[:space:]]*delete-file" contained containedin=vv_r_construct_delete_file
    syn match vv_h_clause_delete_file " status \@=" contained containedin=vv_r_construct_delete_file
    syn match vv_h_clause_delete_file "[=]\@<=define \@=" contained containedin=vv_r_construct_delete_file
    syn match vv_h_clause_delete_file " define \@=" contained containedin=vv_r_construct_delete_file
    hi def link vv_h_clause_delete_file    velyClause
    hi def link vv_h_construct_delete_file    velyConstruct
    hi def link vv_h_print_inline_delete_file    velyConstruct
syn region vv_r_construct_rename_file start="^[[:space:]]*rename-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_rename_file,vv_r_inline_rename_file,vv_r_at
    syn match vv_h_construct_rename_file "^[[:space:]]*rename-file" contained containedin=vv_r_construct_rename_file
    syn match vv_h_clause_rename_file " status \@=" contained containedin=vv_r_construct_rename_file
    syn match vv_h_clause_rename_file " to \@=" contained containedin=vv_r_construct_rename_file
    syn match vv_h_clause_rename_file "[=]\@<=define \@=" contained containedin=vv_r_construct_rename_file
    syn match vv_h_clause_rename_file " define \@=" contained containedin=vv_r_construct_rename_file
    hi def link vv_h_clause_rename_file    velyClause
    hi def link vv_h_construct_rename_file    velyConstruct
    hi def link vv_h_print_inline_rename_file    velyConstruct
syn region vv_r_construct_new_json start="^[[:space:]]*new-json" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_new_json,vv_r_inline_new_json,vv_r_at
    syn match vv_h_construct_new_json "^[[:space:]]*new-json" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " error-position \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " error-text \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " from \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " length \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " max-hash-size \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " node-handler \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " noencode \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " noencode$" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " no-hash \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " no-hash$" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " status \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json "[=]\@<=define \@=" contained containedin=vv_r_construct_new_json
    syn match vv_h_clause_new_json " define \@=" contained containedin=vv_r_construct_new_json
    hi def link vv_h_clause_new_json    velyClause
    hi def link vv_h_construct_new_json    velyConstruct
    hi def link vv_h_print_inline_new_json    velyConstruct
syn region vv_r_construct_delete_json start="^[[:space:]]*delete-json" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_delete_json,vv_r_inline_delete_json,vv_r_at
    syn match vv_h_construct_delete_json "^[[:space:]]*delete-json" contained containedin=vv_r_construct_delete_json
    syn match vv_h_clause_delete_json "[=]\@<=define \@=" contained containedin=vv_r_construct_delete_json
    syn match vv_h_clause_delete_json " define \@=" contained containedin=vv_r_construct_delete_json
    hi def link vv_h_clause_delete_json    velyClause
    hi def link vv_h_construct_delete_json    velyConstruct
    hi def link vv_h_print_inline_delete_json    velyConstruct
syn region vv_r_construct_json_utf8 start="^[[:space:]]*json-utf8" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_json_utf8,vv_r_inline_json_utf8,vv_r_at
    syn match vv_h_construct_json_utf8 "^[[:space:]]*json-utf8" contained containedin=vv_r_construct_json_utf8
    syn match vv_h_clause_json_utf8 " error-text \@=" contained containedin=vv_r_construct_json_utf8
    syn match vv_h_clause_json_utf8 " status \@=" contained containedin=vv_r_construct_json_utf8
    syn match vv_h_clause_json_utf8 "[=]\@<=define \@=" contained containedin=vv_r_construct_json_utf8
    syn match vv_h_clause_json_utf8 " define \@=" contained containedin=vv_r_construct_json_utf8
    hi def link vv_h_clause_json_utf8    velyClause
    hi def link vv_h_construct_json_utf8    velyConstruct
    hi def link vv_h_print_inline_json_utf8    velyConstruct
syn region vv_r_construct_utf8_json start="^[[:space:]]*utf8-json" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_utf8_json,vv_r_inline_utf8_json,vv_r_at
    syn match vv_h_construct_utf8_json "^[[:space:]]*utf8-json" contained containedin=vv_r_construct_utf8_json
    syn match vv_h_clause_utf8_json " error-text \@=" contained containedin=vv_r_construct_utf8_json
    syn match vv_h_clause_utf8_json " length \@=" contained containedin=vv_r_construct_utf8_json
    syn match vv_h_clause_utf8_json " status \@=" contained containedin=vv_r_construct_utf8_json
    syn match vv_h_clause_utf8_json " to \@=" contained containedin=vv_r_construct_utf8_json
    syn match vv_h_clause_utf8_json "[=]\@<=define \@=" contained containedin=vv_r_construct_utf8_json
    syn match vv_h_clause_utf8_json " define \@=" contained containedin=vv_r_construct_utf8_json
    hi def link vv_h_clause_utf8_json    velyClause
    hi def link vv_h_construct_utf8_json    velyConstruct
    hi def link vv_h_print_inline_utf8_json    velyConstruct
syn region vv_r_construct_read_json start="^[[:space:]]*read-json" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_read_json,vv_r_inline_read_json,vv_r_at
    syn match vv_h_construct_read_json "^[[:space:]]*read-json" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " begin \@=" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " begin$" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " key \@=" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " key-list \@=" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " status \@=" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " traverse \@=" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " traverse$" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " type \@=" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " value \@=" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json "[=]\@<=define \@=" contained containedin=vv_r_construct_read_json
    syn match vv_h_clause_read_json " define \@=" contained containedin=vv_r_construct_read_json
    hi def link vv_h_clause_read_json    velyClause
    hi def link vv_h_construct_read_json    velyConstruct
    hi def link vv_h_print_inline_read_json    velyConstruct
syn region vv_r_construct_stat_file start="^[[:space:]]*stat-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_stat_file,vv_r_inline_stat_file,vv_r_at
    syn match vv_h_construct_stat_file "^[[:space:]]*stat-file" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " name \@=" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " name$" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " path \@=" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " path$" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " size \@=" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " size$" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " to \@=" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " type \@=" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " type$" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file "[=]\@<=define \@=" contained containedin=vv_r_construct_stat_file
    syn match vv_h_clause_stat_file " define \@=" contained containedin=vv_r_construct_stat_file
    hi def link vv_h_clause_stat_file    velyClause
    hi def link vv_h_construct_stat_file    velyConstruct
    hi def link vv_h_print_inline_stat_file    velyConstruct
syn region vv_r_construct_decrypt_data start="^[[:space:]]*decrypt-data" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_decrypt_data,vv_r_inline_decrypt_data,vv_r_at
    syn match vv_h_construct_decrypt_data "^[[:space:]]*decrypt-data" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " binary \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " binary$" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " cache \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " cache$" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " cipher \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " clear-cache \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " digest \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " init-vector \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " input-length \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " iterations \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " output-length \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " password \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " salt \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " salt-length \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " to \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data "[=]\@<=define \@=" contained containedin=vv_r_construct_decrypt_data
    syn match vv_h_clause_decrypt_data " define \@=" contained containedin=vv_r_construct_decrypt_data
    hi def link vv_h_clause_decrypt_data    velyClause
    hi def link vv_h_construct_decrypt_data    velyConstruct
    hi def link vv_h_print_inline_decrypt_data    velyConstruct
syn region vv_r_construct_encrypt_data start="^[[:space:]]*encrypt-data" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_encrypt_data,vv_r_inline_encrypt_data,vv_r_at
    syn match vv_h_construct_encrypt_data "^[[:space:]]*encrypt-data" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " binary \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " binary$" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " cache \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " cache$" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " cipher \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " clear-cache \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " digest \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " init-vector \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " input-length \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " iterations \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " output-length \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " password \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " salt \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " salt-length \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " to \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data "[=]\@<=define \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " define \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data "[=]\@<=define \@=" contained containedin=vv_r_construct_encrypt_data
    syn match vv_h_clause_encrypt_data " define \@=" contained containedin=vv_r_construct_encrypt_data
    hi def link vv_h_clause_encrypt_data    velyClause
    hi def link vv_h_construct_encrypt_data    velyConstruct
    hi def link vv_h_print_inline_encrypt_data    velyConstruct
syn region vv_r_construct_trim_string start="^[[:space:]]*trim-string" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_trim_string,vv_r_inline_trim_string,vv_r_at
    syn match vv_h_construct_trim_string "^[[:space:]]*trim-string" contained containedin=vv_r_construct_trim_string
    syn match vv_h_clause_trim_string " length \@=" contained containedin=vv_r_construct_trim_string
    syn match vv_h_clause_trim_string " result \@=" contained containedin=vv_r_construct_trim_string
    syn match vv_h_clause_trim_string "[=]\@<=define \@=" contained containedin=vv_r_construct_trim_string
    syn match vv_h_clause_trim_string " define \@=" contained containedin=vv_r_construct_trim_string
    hi def link vv_h_clause_trim_string    velyClause
    hi def link vv_h_construct_trim_string    velyConstruct
    hi def link vv_h_print_inline_trim_string    velyConstruct
syn region vv_r_construct_encode_web start="^[[:space:]]*encode-web" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_encode_web,vv_r_inline_encode_web,vv_r_at
    syn match vv_h_construct_encode_web "^[[:space:]]*encode-web" contained containedin=vv_r_construct_encode_web
    syn match vv_h_clause_encode_web " input-length \@=" contained containedin=vv_r_construct_encode_web
    syn match vv_h_clause_encode_web " output-length \@=" contained containedin=vv_r_construct_encode_web
    syn match vv_h_clause_encode_web " to \@=" contained containedin=vv_r_construct_encode_web
    syn match vv_h_clause_encode_web "[=]\@<=define \@=" contained containedin=vv_r_construct_encode_web
    syn match vv_h_clause_encode_web " define \@=" contained containedin=vv_r_construct_encode_web
    hi def link vv_h_clause_encode_web    velyClause
    hi def link vv_h_construct_encode_web    velyConstruct
    hi def link vv_h_print_inline_encode_web    velyConstruct
syn region vv_r_construct_encode_url start="^[[:space:]]*encode-url" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_encode_url,vv_r_inline_encode_url,vv_r_at
    syn match vv_h_construct_encode_url "^[[:space:]]*encode-url" contained containedin=vv_r_construct_encode_url
    syn match vv_h_clause_encode_url " input-length \@=" contained containedin=vv_r_construct_encode_url
    syn match vv_h_clause_encode_url " output-length \@=" contained containedin=vv_r_construct_encode_url
    syn match vv_h_clause_encode_url " to \@=" contained containedin=vv_r_construct_encode_url
    syn match vv_h_clause_encode_url "[=]\@<=define \@=" contained containedin=vv_r_construct_encode_url
    syn match vv_h_clause_encode_url " define \@=" contained containedin=vv_r_construct_encode_url
    syn match vv_h_clause_encode_url "[=]\@<=define \@=" contained containedin=vv_r_construct_encode_url
    syn match vv_h_clause_encode_url " define \@=" contained containedin=vv_r_construct_encode_url
    hi def link vv_h_clause_encode_url    velyClause
    hi def link vv_h_construct_encode_url    velyConstruct
    hi def link vv_h_print_inline_encode_url    velyConstruct
syn region vv_r_construct_decode_web start="^[[:space:]]*decode-web" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_decode_web,vv_r_inline_decode_web,vv_r_at
    syn match vv_h_construct_decode_web "^[[:space:]]*decode-web" contained containedin=vv_r_construct_decode_web
    syn match vv_h_clause_decode_web " input-length \@=" contained containedin=vv_r_construct_decode_web
    syn match vv_h_clause_decode_web " output-length \@=" contained containedin=vv_r_construct_decode_web
    syn match vv_h_clause_decode_web "[=]\@<=define \@=" contained containedin=vv_r_construct_decode_web
    syn match vv_h_clause_decode_web " define \@=" contained containedin=vv_r_construct_decode_web
    hi def link vv_h_clause_decode_web    velyClause
    hi def link vv_h_construct_decode_web    velyConstruct
    hi def link vv_h_print_inline_decode_web    velyConstruct
syn region vv_r_construct_decode_url start="^[[:space:]]*decode-url" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_decode_url,vv_r_inline_decode_url,vv_r_at
    syn match vv_h_construct_decode_url "^[[:space:]]*decode-url" contained containedin=vv_r_construct_decode_url
    syn match vv_h_clause_decode_url " input-length \@=" contained containedin=vv_r_construct_decode_url
    syn match vv_h_clause_decode_url " output-length \@=" contained containedin=vv_r_construct_decode_url
    syn match vv_h_clause_decode_url "[=]\@<=define \@=" contained containedin=vv_r_construct_decode_url
    syn match vv_h_clause_decode_url " define \@=" contained containedin=vv_r_construct_decode_url
    syn match vv_h_clause_decode_url "[=]\@<=define \@=" contained containedin=vv_r_construct_decode_url
    syn match vv_h_clause_decode_url " define \@=" contained containedin=vv_r_construct_decode_url
    hi def link vv_h_clause_decode_url    velyClause
    hi def link vv_h_construct_decode_url    velyConstruct
    hi def link vv_h_print_inline_decode_url    velyConstruct
syn region vv_r_construct_decode_hex start="^[[:space:]]*decode-hex" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_decode_hex,vv_r_inline_decode_hex,vv_r_at
    syn match vv_h_construct_decode_hex "^[[:space:]]*decode-hex" contained containedin=vv_r_construct_decode_hex
    syn match vv_h_clause_decode_hex " input-length \@=" contained containedin=vv_r_construct_decode_hex
    syn match vv_h_clause_decode_hex " output-length \@=" contained containedin=vv_r_construct_decode_hex
    syn match vv_h_clause_decode_hex " to \@=" contained containedin=vv_r_construct_decode_hex
    syn match vv_h_clause_decode_hex "[=]\@<=define \@=" contained containedin=vv_r_construct_decode_hex
    syn match vv_h_clause_decode_hex " define \@=" contained containedin=vv_r_construct_decode_hex
    hi def link vv_h_clause_decode_hex    velyClause
    hi def link vv_h_construct_decode_hex    velyConstruct
    hi def link vv_h_print_inline_decode_hex    velyConstruct
syn region vv_r_construct_encode_hex start="^[[:space:]]*encode-hex" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_encode_hex,vv_r_inline_encode_hex,vv_r_at
    syn match vv_h_construct_encode_hex "^[[:space:]]*encode-hex" contained containedin=vv_r_construct_encode_hex
    syn match vv_h_clause_encode_hex " input-length \@=" contained containedin=vv_r_construct_encode_hex
    syn match vv_h_clause_encode_hex " output-length \@=" contained containedin=vv_r_construct_encode_hex
    syn match vv_h_clause_encode_hex " prefix \@=" contained containedin=vv_r_construct_encode_hex
    syn match vv_h_clause_encode_hex " to \@=" contained containedin=vv_r_construct_encode_hex
    syn match vv_h_clause_encode_hex "[=]\@<=define \@=" contained containedin=vv_r_construct_encode_hex
    syn match vv_h_clause_encode_hex " define \@=" contained containedin=vv_r_construct_encode_hex
    hi def link vv_h_clause_encode_hex    velyClause
    hi def link vv_h_construct_encode_hex    velyConstruct
    hi def link vv_h_print_inline_encode_hex    velyConstruct
syn region vv_r_construct_encode_base64 start="^[[:space:]]*encode-base64" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_encode_base64,vv_r_inline_encode_base64,vv_r_at
    syn match vv_h_construct_encode_base64 "^[[:space:]]*encode-base64" contained containedin=vv_r_construct_encode_base64
    syn match vv_h_clause_encode_base64 " input-length \@=" contained containedin=vv_r_construct_encode_base64
    syn match vv_h_clause_encode_base64 " output-length \@=" contained containedin=vv_r_construct_encode_base64
    syn match vv_h_clause_encode_base64 " to \@=" contained containedin=vv_r_construct_encode_base64
    syn match vv_h_clause_encode_base64 "[=]\@<=define \@=" contained containedin=vv_r_construct_encode_base64
    syn match vv_h_clause_encode_base64 " define \@=" contained containedin=vv_r_construct_encode_base64
    hi def link vv_h_clause_encode_base64    velyClause
    hi def link vv_h_construct_encode_base64    velyConstruct
    hi def link vv_h_print_inline_encode_base64    velyConstruct
syn region vv_r_construct_decode_base64 start="^[[:space:]]*decode-base64" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_decode_base64,vv_r_inline_decode_base64,vv_r_at
    syn match vv_h_construct_decode_base64 "^[[:space:]]*decode-base64" contained containedin=vv_r_construct_decode_base64
    syn match vv_h_clause_decode_base64 " input-length \@=" contained containedin=vv_r_construct_decode_base64
    syn match vv_h_clause_decode_base64 " output-length \@=" contained containedin=vv_r_construct_decode_base64
    syn match vv_h_clause_decode_base64 " to \@=" contained containedin=vv_r_construct_decode_base64
    syn match vv_h_clause_decode_base64 "[=]\@<=define \@=" contained containedin=vv_r_construct_decode_base64
    syn match vv_h_clause_decode_base64 " define \@=" contained containedin=vv_r_construct_decode_base64
    hi def link vv_h_clause_decode_base64    velyClause
    hi def link vv_h_construct_decode_base64    velyConstruct
    hi def link vv_h_print_inline_decode_base64    velyConstruct
syn region vv_r_construct_hash_string start="^[[:space:]]*hash-string" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_hash_string,vv_r_inline_hash_string,vv_r_at
    syn match vv_h_construct_hash_string "^[[:space:]]*hash-string" contained containedin=vv_r_construct_hash_string
    syn match vv_h_clause_hash_string " binary \@=" contained containedin=vv_r_construct_hash_string
    syn match vv_h_clause_hash_string " binary$" contained containedin=vv_r_construct_hash_string
    syn match vv_h_clause_hash_string " digest \@=" contained containedin=vv_r_construct_hash_string
    syn match vv_h_clause_hash_string " output-length \@=" contained containedin=vv_r_construct_hash_string
    syn match vv_h_clause_hash_string " to \@=" contained containedin=vv_r_construct_hash_string
    syn match vv_h_clause_hash_string "[=]\@<=define \@=" contained containedin=vv_r_construct_hash_string
    syn match vv_h_clause_hash_string " define \@=" contained containedin=vv_r_construct_hash_string
    hi def link vv_h_clause_hash_string    velyClause
    hi def link vv_h_construct_hash_string    velyConstruct
    hi def link vv_h_print_inline_hash_string    velyConstruct
syn region vv_r_construct_derive_key start="^[[:space:]]*derive-key" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_derive_key,vv_r_inline_derive_key,vv_r_at
    syn match vv_h_construct_derive_key "^[[:space:]]*derive-key" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " binary \@=" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " binary$" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " digest \@=" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " from \@=" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " from-length \@=" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " iterations \@=" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " length \@=" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " salt \@=" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " salt-length \@=" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key "[=]\@<=define \@=" contained containedin=vv_r_construct_derive_key
    syn match vv_h_clause_derive_key " define \@=" contained containedin=vv_r_construct_derive_key
    hi def link vv_h_clause_derive_key    velyClause
    hi def link vv_h_construct_derive_key    velyConstruct
    hi def link vv_h_print_inline_derive_key    velyConstruct
syn region vv_r_construct_count_substring start="^[[:space:]]*count-substring" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_count_substring,vv_r_inline_count_substring,vv_r_at
    syn match vv_h_construct_count_substring "^[[:space:]]*count-substring" contained containedin=vv_r_construct_count_substring
    syn match vv_h_clause_count_substring " case-insensitive \@=" contained containedin=vv_r_construct_count_substring
    syn match vv_h_clause_count_substring " case-insensitive$" contained containedin=vv_r_construct_count_substring
    syn match vv_h_clause_count_substring " in \@=" contained containedin=vv_r_construct_count_substring
    syn match vv_h_clause_count_substring " to \@=" contained containedin=vv_r_construct_count_substring
    syn match vv_h_clause_count_substring "[=]\@<=define \@=" contained containedin=vv_r_construct_count_substring
    syn match vv_h_clause_count_substring " define \@=" contained containedin=vv_r_construct_count_substring
    hi def link vv_h_clause_count_substring    velyClause
    hi def link vv_h_construct_count_substring    velyConstruct
    hi def link vv_h_print_inline_count_substring    velyConstruct
syn region vv_r_construct_lower_string start="^[[:space:]]*lower-string" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_lower_string,vv_r_inline_lower_string,vv_r_at
    syn match vv_h_construct_lower_string "^[[:space:]]*lower-string" contained containedin=vv_r_construct_lower_string
    syn match vv_h_clause_lower_string "[=]\@<=define \@=" contained containedin=vv_r_construct_lower_string
    syn match vv_h_clause_lower_string " define \@=" contained containedin=vv_r_construct_lower_string
    hi def link vv_h_clause_lower_string    velyClause
    hi def link vv_h_construct_lower_string    velyConstruct
    hi def link vv_h_print_inline_lower_string    velyConstruct
syn region vv_r_construct_upper_string start="^[[:space:]]*upper-string" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_upper_string,vv_r_inline_upper_string,vv_r_at
    syn match vv_h_construct_upper_string "^[[:space:]]*upper-string" contained containedin=vv_r_construct_upper_string
    syn match vv_h_clause_upper_string "[=]\@<=define \@=" contained containedin=vv_r_construct_upper_string
    syn match vv_h_clause_upper_string " define \@=" contained containedin=vv_r_construct_upper_string
    hi def link vv_h_clause_upper_string    velyClause
    hi def link vv_h_construct_upper_string    velyConstruct
    hi def link vv_h_print_inline_upper_string    velyConstruct
syn region vv_r_construct_random_crypto start="^[[:space:]]*random-crypto" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_random_crypto,vv_r_inline_random_crypto,vv_r_at
    syn match vv_h_construct_random_crypto "^[[:space:]]*random-crypto" contained containedin=vv_r_construct_random_crypto
    syn match vv_h_clause_random_crypto " length \@=" contained containedin=vv_r_construct_random_crypto
    syn match vv_h_clause_random_crypto " to \@=" contained containedin=vv_r_construct_random_crypto
    syn match vv_h_clause_random_crypto "[=]\@<=define \@=" contained containedin=vv_r_construct_random_crypto
    syn match vv_h_clause_random_crypto " define \@=" contained containedin=vv_r_construct_random_crypto
    hi def link vv_h_clause_random_crypto    velyClause
    hi def link vv_h_construct_random_crypto    velyConstruct
    hi def link vv_h_print_inline_random_crypto    velyConstruct
syn region vv_r_construct_random_string start="^[[:space:]]*random-string" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_random_string,vv_r_inline_random_string,vv_r_at
    syn match vv_h_construct_random_string "^[[:space:]]*random-string" contained containedin=vv_r_construct_random_string
    syn match vv_h_clause_random_string " binary \@=" contained containedin=vv_r_construct_random_string
    syn match vv_h_clause_random_string " binary$" contained containedin=vv_r_construct_random_string
    syn match vv_h_clause_random_string " length \@=" contained containedin=vv_r_construct_random_string
    syn match vv_h_clause_random_string " number \@=" contained containedin=vv_r_construct_random_string
    syn match vv_h_clause_random_string " number$" contained containedin=vv_r_construct_random_string
    syn match vv_h_clause_random_string " to \@=" contained containedin=vv_r_construct_random_string
    syn match vv_h_clause_random_string "[=]\@<=define \@=" contained containedin=vv_r_construct_random_string
    syn match vv_h_clause_random_string " define \@=" contained containedin=vv_r_construct_random_string
    hi def link vv_h_clause_random_string    velyClause
    hi def link vv_h_construct_random_string    velyConstruct
    hi def link vv_h_print_inline_random_string    velyConstruct
syn region vv_r_construct_unlock_file start="^[[:space:]]*unlock-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_unlock_file,vv_r_inline_unlock_file,vv_r_at
    syn match vv_h_construct_unlock_file "^[[:space:]]*unlock-file" contained containedin=vv_r_construct_unlock_file
    syn match vv_h_clause_unlock_file " id \@=" contained containedin=vv_r_construct_unlock_file
    syn match vv_h_clause_unlock_file "[=]\@<=define \@=" contained containedin=vv_r_construct_unlock_file
    syn match vv_h_clause_unlock_file " define \@=" contained containedin=vv_r_construct_unlock_file
    hi def link vv_h_clause_unlock_file    velyClause
    hi def link vv_h_construct_unlock_file    velyConstruct
    hi def link vv_h_print_inline_unlock_file    velyConstruct
syn region vv_r_construct_lock_file start="^[[:space:]]*lock-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_lock_file,vv_r_inline_lock_file,vv_r_at
    syn match vv_h_construct_lock_file "^[[:space:]]*lock-file" contained containedin=vv_r_construct_lock_file
    syn match vv_h_clause_lock_file " id \@=" contained containedin=vv_r_construct_lock_file
    syn match vv_h_clause_lock_file " status \@=" contained containedin=vv_r_construct_lock_file
    syn match vv_h_clause_lock_file "[=]\@<=define \@=" contained containedin=vv_r_construct_lock_file
    syn match vv_h_clause_lock_file " define \@=" contained containedin=vv_r_construct_lock_file
    hi def link vv_h_clause_lock_file    velyClause
    hi def link vv_h_construct_lock_file    velyConstruct
    hi def link vv_h_print_inline_lock_file    velyConstruct
syn region vv_r_construct_send_file start="^[[:space:]]*send-file" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_send_file,vv_r_inline_send_file,vv_r_at
    syn match vv_h_construct_send_file "^[[:space:]]*send-file" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " cache-control \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " content-type \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " custom \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " download \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " download$" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " etag \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " etag$" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " file-name \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " headers \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " no-cache \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " no-cache$" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " status-id \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " status-text \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file "[=]\@<=define \@=" contained containedin=vv_r_construct_send_file
    syn match vv_h_clause_send_file " define \@=" contained containedin=vv_r_construct_send_file
    hi def link vv_h_clause_send_file    velyClause
    hi def link vv_h_construct_send_file    velyConstruct
    hi def link vv_h_print_inline_send_file    velyConstruct
syn region vv_r_construct_exec_program start="^[[:space:]]*exec-program" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_exec_program,vv_r_inline_exec_program,vv_r_at
    syn match vv_h_construct_exec_program "^[[:space:]]*exec-program" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " args \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " error \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " error-file \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " input \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " input-file \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " input-length \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " output \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " output-file \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " output-length \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " status \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program "[=]\@<=define \@=" contained containedin=vv_r_construct_exec_program
    syn match vv_h_clause_exec_program " define \@=" contained containedin=vv_r_construct_exec_program
    hi def link vv_h_clause_exec_program    velyClause
    hi def link vv_h_construct_exec_program    velyConstruct
    hi def link vv_h_print_inline_exec_program    velyConstruct
syn region vv_r_construct_out_header start="^[[:space:]]*out-header" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_out_header,vv_r_inline_out_header,vv_r_at
    syn match vv_h_construct_out_header "^[[:space:]]*out-header" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " cache-control \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " content-type \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " custom \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " default \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " default$" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " download \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " download$" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " etag \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " etag$" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " file-name \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " no-cache \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " no-cache$" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " status-id \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " status-text \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " use \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header "[=]\@<=define \@=" contained containedin=vv_r_construct_out_header
    syn match vv_h_clause_out_header " define \@=" contained containedin=vv_r_construct_out_header
    hi def link vv_h_clause_out_header    velyClause
    hi def link vv_h_construct_out_header    velyConstruct
    hi def link vv_h_print_inline_out_header    velyConstruct
syn region vv_r_construct_silent_header start="^[[:space:]]*silent-header" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_silent_header,vv_r_inline_silent_header,vv_r_at
    syn match vv_h_construct_silent_header "^[[:space:]]*silent-header" contained containedin=vv_r_construct_silent_header
    syn match vv_h_clause_silent_header "[=]\@<=define \@=" contained containedin=vv_r_construct_silent_header
    syn match vv_h_clause_silent_header " define \@=" contained containedin=vv_r_construct_silent_header
    hi def link vv_h_clause_silent_header    velyClause
    hi def link vv_h_construct_silent_header    velyConstruct
    hi def link vv_h_print_inline_silent_header    velyConstruct
syn region vv_r_construct_exit_code start="^[[:space:]]*exit-code" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_exit_code,vv_r_inline_exit_code,vv_r_at
    syn match vv_h_construct_exit_code "^[[:space:]]*exit-code" contained containedin=vv_r_construct_exit_code
    syn match vv_h_clause_exit_code "[=]\@<=define \@=" contained containedin=vv_r_construct_exit_code
    syn match vv_h_clause_exit_code " define \@=" contained containedin=vv_r_construct_exit_code
    hi def link vv_h_clause_exit_code    velyClause
    hi def link vv_h_construct_exit_code    velyConstruct
    hi def link vv_h_print_inline_exit_code    velyConstruct
syn region vv_r_construct_p_web start="^[[:space:]]*p-web" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_p_web,vv_r_inline_p_web,vv_r_at
    syn match vv_h_construct_p_web "^[[:space:]]*p-web" contained containedin=vv_r_construct_p_web
    syn match vv_h_clause_p_web "[=]\@<=define \@=" contained containedin=vv_r_construct_p_web
    syn match vv_h_clause_p_web " define \@=" contained containedin=vv_r_construct_p_web
    syn match vv_h_print_inline_p_web " define \@=" contained containedin=vv_r_inline_p_web
    syn match vv_h_print_inline_p_web " define \@=" contained containedin=vv_r_inline_p_web
    syn region vv_r_inline_p_web start="<<[[:space:]]*p-web \@=" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_p_web '<<[[:space:]]*p-web \@=' contained containedin=vv_r_inline_p_web
    syn match vv_h_print_inline_p_web '>>' contained containedin=vv_r_inline_p_web
    hi def link vv_h_clause_p_web    velyClause
    hi def link vv_h_construct_p_web    velyConstruct
    hi def link vv_h_print_inline_p_web    velyConstruct
syn region vv_r_construct_p_url start="^[[:space:]]*p-url" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_p_url,vv_r_inline_p_url,vv_r_at
    syn match vv_h_construct_p_url "^[[:space:]]*p-url" contained containedin=vv_r_construct_p_url
    syn match vv_h_clause_p_url "[=]\@<=define \@=" contained containedin=vv_r_construct_p_url
    syn match vv_h_clause_p_url " define \@=" contained containedin=vv_r_construct_p_url
    syn match vv_h_print_inline_p_url " define \@=" contained containedin=vv_r_inline_p_url
    syn match vv_h_print_inline_p_url " define \@=" contained containedin=vv_r_inline_p_url
    syn region vv_r_inline_p_url start="<<[[:space:]]*p-url \@=" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_p_url '<<[[:space:]]*p-url \@=' contained containedin=vv_r_inline_p_url
    syn match vv_h_print_inline_p_url '>>' contained containedin=vv_r_inline_p_url
    hi def link vv_h_clause_p_url    velyClause
    hi def link vv_h_construct_p_url    velyConstruct
    hi def link vv_h_print_inline_p_url    velyConstruct
syn region vv_r_construct_p_num start="^[[:space:]]*p-num" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_p_num,vv_r_inline_p_num,vv_r_at
    syn match vv_h_construct_p_num "^[[:space:]]*p-num" contained containedin=vv_r_construct_p_num
    syn match vv_h_clause_p_num "[=]\@<=define \@=" contained containedin=vv_r_construct_p_num
    syn match vv_h_clause_p_num " define \@=" contained containedin=vv_r_construct_p_num
    syn match vv_h_print_inline_p_num " define \@=" contained containedin=vv_r_inline_p_num
    syn match vv_h_print_inline_p_num " define \@=" contained containedin=vv_r_inline_p_num
    syn region vv_r_inline_p_num start="<<[[:space:]]*p-num \@=" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_p_num '<<[[:space:]]*p-num \@=' contained containedin=vv_r_inline_p_num
    syn match vv_h_print_inline_p_num '>>' contained containedin=vv_r_inline_p_num
    hi def link vv_h_clause_p_num    velyClause
    hi def link vv_h_construct_p_num    velyConstruct
    hi def link vv_h_print_inline_p_num    velyConstruct
syn region vv_r_construct_p_path start="^[[:space:]]*p-path" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_p_path,vv_r_inline_p_path,vv_r_at
    syn match vv_h_construct_p_path "^[[:space:]]*p-path" contained containedin=vv_r_construct_p_path
    syn match vv_h_clause_p_path "[=]\@<=define \@=" contained containedin=vv_r_construct_p_path
    syn match vv_h_clause_p_path " define \@=" contained containedin=vv_r_construct_p_path
    syn match vv_h_print_inline_p_path " define \@=" contained containedin=vv_r_inline_p_path
    syn match vv_h_print_inline_p_path " define \@=" contained containedin=vv_r_inline_p_path
    syn region vv_r_inline_p_path start="<<[[:space:]]*p-path" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_p_path '<<[[:space:]]*p-path' contained containedin=vv_r_inline_p_path
    syn match vv_h_print_inline_p_path '>>' contained containedin=vv_r_inline_p_path
    hi def link vv_h_clause_p_path    velyClause
    hi def link vv_h_construct_p_path    velyConstruct
    hi def link vv_h_print_inline_p_path    velyConstruct
syn region vv_r_construct_p_dbl start="^[[:space:]]*p-dbl" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_p_dbl,vv_r_inline_p_dbl,vv_r_at
    syn match vv_h_construct_p_dbl "^[[:space:]]*p-dbl" contained containedin=vv_r_construct_p_dbl
    syn match vv_h_clause_p_dbl "[=]\@<=define \@=" contained containedin=vv_r_construct_p_dbl
    syn match vv_h_clause_p_dbl " define \@=" contained containedin=vv_r_construct_p_dbl
    syn match vv_h_print_inline_p_dbl " define \@=" contained containedin=vv_r_inline_p_dbl
    syn match vv_h_print_inline_p_dbl " define \@=" contained containedin=vv_r_inline_p_dbl
    syn region vv_r_inline_p_dbl start="<<[[:space:]]*p-dbl \@=" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_p_dbl '<<[[:space:]]*p-dbl \@=' contained containedin=vv_r_inline_p_dbl
    syn match vv_h_print_inline_p_dbl '>>' contained containedin=vv_r_inline_p_dbl
    hi def link vv_h_clause_p_dbl    velyClause
    hi def link vv_h_construct_p_dbl    velyConstruct
    hi def link vv_h_print_inline_p_dbl    velyConstruct
syn region vv_r_construct_p_out start="^[[:space:]]*p-out" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_p_out,vv_r_inline_p_out,vv_r_at
    syn match vv_h_construct_p_out "^[[:space:]]*p-out" contained containedin=vv_r_construct_p_out
    syn match vv_h_clause_p_out "[=]\@<=define \@=" contained containedin=vv_r_construct_p_out
    syn match vv_h_clause_p_out " define \@=" contained containedin=vv_r_construct_p_out
    syn match vv_h_print_inline_p_out " define \@=" contained containedin=vv_r_inline_p_out
    syn match vv_h_print_inline_p_out " define \@=" contained containedin=vv_r_inline_p_out
    syn region vv_r_inline_p_out start="<<[[:space:]]*p-out \@=" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_p_out '<<[[:space:]]*p-out \@=' contained containedin=vv_r_inline_p_out
    syn match vv_h_print_inline_p_out '>>' contained containedin=vv_r_inline_p_out
    hi def link vv_h_clause_p_out    velyClause
    hi def link vv_h_construct_p_out    velyConstruct
    hi def link vv_h_print_inline_p_out    velyConstruct
syn region vv_r_construct_run_query start="^[[:space:]]*run-query" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_run_query,vv_r_inline_run_query,vv_r_at
    syn match vv_h_construct_run_query "^[[:space:]]*run-query" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " affected-rows \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " @ \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " @$" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " : \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " :$" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " column-count \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " column-data \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " column-names \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " error \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " error-text \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " input \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " name \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " no-loop \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " no-loop$" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " on-error-continue \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " on-error-continue$" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " on-error-exit \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " on-error-exit$" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " output \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " row-count \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " unknown-output \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " unknown-output$" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query "[=]\@<=define \@=" contained containedin=vv_r_construct_run_query
    syn match vv_h_clause_run_query " define \@=" contained containedin=vv_r_construct_run_query
    hi def link vv_h_clause_run_query    velyClause
    hi def link vv_h_construct_run_query    velyConstruct
    hi def link vv_h_print_inline_run_query    velyConstruct
syn region vv_r_construct_run_prepared_query start="^[[:space:]]*run-prepared-query" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_run_prepared_query,vv_r_inline_run_prepared_query,vv_r_at
    syn match vv_h_construct_run_prepared_query "^[[:space:]]*run-prepared-query" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " affected-rows \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " @ \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " @$" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " : \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " :$" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " column-count \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " column-data \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " column-names \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " error \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " error-text \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " input \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " name \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " no-loop \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " no-loop$" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " on-error-continue \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " on-error-continue$" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " on-error-exit \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " on-error-exit$" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " output \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " row-count \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " unknown-output \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " unknown-output$" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query "[=]\@<=define \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " define \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query "[=]\@<=define \@=" contained containedin=vv_r_construct_run_prepared_query
    syn match vv_h_clause_run_prepared_query " define \@=" contained containedin=vv_r_construct_run_prepared_query
    hi def link vv_h_clause_run_prepared_query    velyClause
    hi def link vv_h_construct_run_prepared_query    velyConstruct
    hi def link vv_h_print_inline_run_prepared_query    velyConstruct
syn region vv_r_construct_query_result start="^[[:space:]]*query-result" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_query_result,vv_r_inline_query_result,vv_r_at
    syn match vv_h_construct_query_result "^[[:space:]]*query-result" contained containedin=vv_r_construct_query_result
    syn match vv_h_clause_query_result " length \@=" contained containedin=vv_r_construct_query_result
    syn match vv_h_print_inline_query_result " length \@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_print_inline_query_result " define \@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_clause_query_result " noencode \@=" contained containedin=vv_r_construct_query_result
    syn match vv_h_clause_query_result " noencode$" contained containedin=vv_r_construct_query_result
    syn match vv_h_print_inline_query_result " noencode\(>>\)\@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_print_inline_query_result " noencode \@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_clause_query_result " to \@=" contained containedin=vv_r_construct_query_result
    syn match vv_h_print_inline_query_result " to \@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_print_inline_query_result " define \@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_clause_query_result " urlencode \@=" contained containedin=vv_r_construct_query_result
    syn match vv_h_clause_query_result " urlencode$" contained containedin=vv_r_construct_query_result
    syn match vv_h_print_inline_query_result " urlencode\(>>\)\@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_print_inline_query_result " urlencode \@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_clause_query_result " webencode \@=" contained containedin=vv_r_construct_query_result
    syn match vv_h_clause_query_result " webencode$" contained containedin=vv_r_construct_query_result
    syn match vv_h_print_inline_query_result " webencode\(>>\)\@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_print_inline_query_result " webencode \@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_clause_query_result "[=]\@<=define \@=" contained containedin=vv_r_construct_query_result
    syn match vv_h_clause_query_result " define \@=" contained containedin=vv_r_construct_query_result
    syn match vv_h_print_inline_query_result " define \@=" contained containedin=vv_r_inline_query_result
    syn match vv_h_print_inline_query_result " define \@=" contained containedin=vv_r_inline_query_result
    syn region vv_r_inline_query_result start="<<[[:space:]]*query-result \@=" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_query_result '<<[[:space:]]*query-result \@=' contained containedin=vv_r_inline_query_result
    syn match vv_h_print_inline_query_result '>>' contained containedin=vv_r_inline_query_result
    hi def link vv_h_clause_query_result    velyClause
    hi def link vv_h_construct_query_result    velyConstruct
    hi def link vv_h_print_inline_query_result    velyConstruct
syn region vv_r_construct_delete_query start="^[[:space:]]*delete-query" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_delete_query,vv_r_inline_delete_query,vv_r_at
    syn match vv_h_construct_delete_query "^[[:space:]]*delete-query" contained containedin=vv_r_construct_delete_query
    syn match vv_h_clause_delete_query " skip-data \@=" contained containedin=vv_r_construct_delete_query
    syn match vv_h_clause_delete_query " skip-data$" contained containedin=vv_r_construct_delete_query
    syn match vv_h_clause_delete_query "[=]\@<=define \@=" contained containedin=vv_r_construct_delete_query
    syn match vv_h_clause_delete_query " define \@=" contained containedin=vv_r_construct_delete_query
    hi def link vv_h_clause_delete_query    velyClause
    hi def link vv_h_construct_delete_query    velyConstruct
    hi def link vv_h_print_inline_delete_query    velyConstruct
syn region vv_r_construct_current_row start="^[[:space:]]*current-row" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_current_row,vv_r_inline_current_row,vv_r_at
    syn match vv_h_construct_current_row "^[[:space:]]*current-row" contained containedin=vv_r_construct_current_row
    syn match vv_h_clause_current_row " to \@=" contained containedin=vv_r_construct_current_row
    syn match vv_h_print_inline_current_row " to \@=" contained containedin=vv_r_inline_current_row
    syn match vv_h_print_inline_current_row " define \@=" contained containedin=vv_r_inline_current_row
    syn match vv_h_clause_current_row "[=]\@<=define \@=" contained containedin=vv_r_construct_current_row
    syn match vv_h_clause_current_row " define \@=" contained containedin=vv_r_construct_current_row
    syn match vv_h_print_inline_current_row " define \@=" contained containedin=vv_r_inline_current_row
    syn match vv_h_print_inline_current_row " define \@=" contained containedin=vv_r_inline_current_row
    syn region vv_r_inline_current_row start="<<[[:space:]]*current-row" skip="\\[[:space:]]*$" end=">>" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat contained containedin=vv_r_at keepend
    syn match vv_h_print_inline_current_row '<<[[:space:]]*current-row' contained containedin=vv_r_inline_current_row
    syn match vv_h_print_inline_current_row '>>' contained containedin=vv_r_inline_current_row
    hi def link vv_h_clause_current_row    velyClause
    hi def link vv_h_construct_current_row    velyConstruct
    hi def link vv_h_print_inline_current_row    velyConstruct
syn region vv_r_construct_begin_transaction start="^[[:space:]]*begin-transaction" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_begin_transaction,vv_r_inline_begin_transaction,vv_r_at
    syn match vv_h_construct_begin_transaction "^[[:space:]]*begin-transaction" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction " @ \@=" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction " @$" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction " error \@=" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction " error-text \@=" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction " on-error-continue \@=" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction " on-error-continue$" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction " on-error-exit \@=" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction " on-error-exit$" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction "[=]\@<=define \@=" contained containedin=vv_r_construct_begin_transaction
    syn match vv_h_clause_begin_transaction " define \@=" contained containedin=vv_r_construct_begin_transaction
    hi def link vv_h_clause_begin_transaction    velyClause
    hi def link vv_h_construct_begin_transaction    velyConstruct
    hi def link vv_h_print_inline_begin_transaction    velyConstruct
syn region vv_r_construct_commit_transaction start="^[[:space:]]*commit-transaction" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_commit_transaction,vv_r_inline_commit_transaction,vv_r_at
    syn match vv_h_construct_commit_transaction "^[[:space:]]*commit-transaction" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction " @ \@=" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction " @$" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction " error \@=" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction " error-text \@=" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction " on-error-continue \@=" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction " on-error-continue$" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction " on-error-exit \@=" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction " on-error-exit$" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction "[=]\@<=define \@=" contained containedin=vv_r_construct_commit_transaction
    syn match vv_h_clause_commit_transaction " define \@=" contained containedin=vv_r_construct_commit_transaction
    hi def link vv_h_clause_commit_transaction    velyClause
    hi def link vv_h_construct_commit_transaction    velyConstruct
    hi def link vv_h_print_inline_commit_transaction    velyConstruct
syn region vv_r_construct_on_error start="^[[:space:]]*on-error" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_on_error,vv_r_inline_on_error,vv_r_at
    syn match vv_h_construct_on_error "^[[:space:]]*on-error" contained containedin=vv_r_construct_on_error
    syn match vv_h_clause_on_error " @ \@=" contained containedin=vv_r_construct_on_error
    syn match vv_h_clause_on_error " @$" contained containedin=vv_r_construct_on_error
    syn match vv_h_clause_on_error " continue \@=" contained containedin=vv_r_construct_on_error
    syn match vv_h_clause_on_error " continue$" contained containedin=vv_r_construct_on_error
    syn match vv_h_clause_on_error " exit \@=" contained containedin=vv_r_construct_on_error
    syn match vv_h_clause_on_error " exit$" contained containedin=vv_r_construct_on_error
    syn match vv_h_clause_on_error "[=]\@<=define \@=" contained containedin=vv_r_construct_on_error
    syn match vv_h_clause_on_error " define \@=" contained containedin=vv_r_construct_on_error
    hi def link vv_h_clause_on_error    velyClause
    hi def link vv_h_construct_on_error    velyConstruct
    hi def link vv_h_print_inline_on_error    velyConstruct
syn region vv_r_construct_rollback_transaction start="^[[:space:]]*rollback-transaction" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_rollback_transaction,vv_r_inline_rollback_transaction,vv_r_at
    syn match vv_h_construct_rollback_transaction "^[[:space:]]*rollback-transaction" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction " @ \@=" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction " @$" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction " error \@=" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction " error-text \@=" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction " on-error-continue \@=" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction " on-error-continue$" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction " on-error-exit \@=" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction " on-error-exit$" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction "[=]\@<=define \@=" contained containedin=vv_r_construct_rollback_transaction
    syn match vv_h_clause_rollback_transaction " define \@=" contained containedin=vv_r_construct_rollback_transaction
    hi def link vv_h_clause_rollback_transaction    velyClause
    hi def link vv_h_construct_rollback_transaction    velyConstruct
    hi def link vv_h_print_inline_rollback_transaction    velyConstruct
syn region vv_r_construct_end_query start="^[[:space:]]*end-query" skip="\\[[:space:]]*$" end="$" contains=cString,cNumbers,cOperator,cType,cConstant,cFormat,cComment,cCommentL keepend
    syn match vv_h_other '[a-zA-Z0-9]\+' contained containedin=vv_r_construct_end_query,vv_r_inline_end_query,vv_r_at
    syn match vv_h_construct_end_query "^[[:space:]]*end-query" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " @ \@=" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " @$" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " error \@=" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " error-text \@=" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " on-error-continue \@=" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " on-error-continue$" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " on-error-exit \@=" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " on-error-exit$" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query "[=]\@<=define \@=" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " define \@=" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query "[=]\@<=define \@=" contained containedin=vv_r_construct_end_query
    syn match vv_h_clause_end_query " define \@=" contained containedin=vv_r_construct_end_query
    hi def link vv_h_clause_end_query    velyClause
    hi def link vv_h_construct_end_query    velyConstruct
    hi def link vv_h_print_inline_end_query    velyConstruct
