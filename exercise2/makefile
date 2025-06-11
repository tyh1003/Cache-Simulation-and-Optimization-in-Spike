.DEFAULT_GOAL: check
.PHONY: check
check: checkex2-1 checkex2-2

.PHONY: checkex2-1
checkex2-1:
	@printf "=============== Exercise 2-1 ===============\n"
	@python3 test_exercise2_1.py

.PHONY: checkex2-2
checkex2-2:
	@printf "=============== Exercise 2-2 ===============\n"
	@python3 test_exercise2_2.py

.PHONY: diffex2-1
diffex2-1:
	@git diff --color-words --no-index -U0 -G. exercise2_1/ans.output exercise2_1/stu.output

.PHONY: diffex2-2
diffex2-2:
	@git diff --color-words --no-index -U0 -G. exercise2_2/ans.output exercise2_2/stu.output

.PHONY: clean
clean:
	$(RM) exercise2_1/*.out* exercise2_2/*.out*