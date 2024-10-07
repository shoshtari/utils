build_dir = "./tmp"

format:
	@black .
	@find . -path  ./venv -prune -o -name "*.py" -type f -print  -exec autopep8 -i {} \;
	@autoflake .
	@isort .
