#pragma once
#include <cstdio>
#include <optional>
#include <string>
#include <functional>
namespace a_c_compiler {

  struct logger {
		static constexpr std::size_t indent_width = 1;
    static void indent() noexcept;
    static void incr_indent() noexcept;
    static void decr_indent() noexcept;
  };

	struct scope_logger {
		std::string scope_name;
		std::optional<std::string_view> logfile;
		scope_logger(std::string scope_name,
         std::function<void()>&& entry_callback,
		     std::optional<std::string_view> logfile = std::nullopt) noexcept
		: logfile(logfile), scope_name(scope_name) {
			// TODO: logfile handling
      logger::indent();
			fprintf(stderr, "%s", scope_name.c_str());
      entry_callback();
      fprintf(stderr, "\n");
      logger::incr_indent();
		}
		~scope_logger() noexcept {
      logger::decr_indent();
      logger::indent();
			fprintf(stderr, "%s\n", scope_name.c_str());
		}

	private:
		void indent() const noexcept {
      logger::indent();
		}
	};
} // namespace a_c_compiler
