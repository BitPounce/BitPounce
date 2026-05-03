#pragma once

#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <spdlog/sinks/base_sink.h>

#include <glm/glm.hpp>

namespace BitPounce
{
	// =========================
	// Variable (bound to real data)
	// =========================
	struct Variable
	{
		std::string name;
		std::function<bool(const std::string&)> setter;
		std::function<std::string()> getter;
	};

	// =========================
	// Command
	// =========================
	struct Command
	{
		std::string name;
		std::function<void(const std::vector<std::string>&)> func;
		std::string help;
	};

	// =========================
	// Utility parsing
	// =========================
	class Parser
	{
	public:
		static std::vector<std::string> Tokenize(const std::string& line)
		{
			std::istringstream iss(line);
			std::vector<std::string> tokens;
			std::string token;

			while (iss >> token)
				tokens.push_back(token);

			return tokens;
		}

		template<typename T>
		static bool Parse(const std::string& s, T& out)
		{
			std::istringstream iss(s);
		
			if (!(iss >> out))
				return false;
		
			// ensure full consumption (optional but recommended)
			return iss.eof();
		}

		static bool Parse(const std::string& s, bool& out)
		{
			if (s == "true" || s == "1") { out = true; return true; }
			if (s == "false" || s == "0") { out = false; return true; }
			return false;
		}
		/*
		static bool Parse(const std::string& s, glm::vec2& out)
		{
			std::istringstream iss(s);
			return static_cast<bool>(iss >> out.x >> out.y);
		}

		static bool Parse(const std::string& s, glm::vec3& out)
		{	
   			std::istringstream iss(s);
   			return static_cast<bool>(iss >> out.x >> out.y >> out.z);
		}	

		static bool Parse(const std::string& s, glm::vec4& out)
	{
		std::istringstream iss(s);
		return static_cast<bool>(iss >> out.x >> out.y >> out.z >> out.w);
	}*/
		template<int L, typename T, glm::qualifier Q>
		static bool Parse(const std::string& s, glm::vec<L, T, Q>& v)
		{
			std::istringstream iss(s);
			bool re = false;
			for (size_t i = 0; i < L; i++)
			{
				re = static_cast<bool>(iss >> v[i]);
			}
			return re;
		}

		template<typename T>
		static std::string ToString(const T& value)
		{
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}
		/*

		static std::string ToString(const glm::vec2& v)
		{
			return std::to_string(v.x) + " " + std::to_string(v.y);
		}

		static std::string ToString(const glm::vec3& v)
		{
			return std::to_string(v.x) + " " +
				   std::to_string(v.y) + " " +
				   std::to_string(v.z);
		}

		static std::string ToString(const glm::vec4& v)
		{
			return std::to_string(v.x) + " " +
				   std::to_string(v.y) + " " +
				   std::to_string(v.z) + " " +
				   std::to_string(v.w);
		}*/
		template<int L, typename T, glm::qualifier Q>
		static std::string ToString(const glm::vec<L, T, Q>& v)
		{
			std::string ret = std::string();
			for (size_t i = 0; i < L; i++)
			{
				ret += std::to_string(v[i]) + ' ';
			}
			
			return ret;
		}

	};

	// =========================
	// Variable Factory (binding)
	// =========================
	class VariableFactory
	{
	public:
		template<typename T>
		static Variable Bind(const std::string& name, T& ref)
		{
			Variable var;
			var.name = name;

			var.setter = [&ref](const std::string& value)
			{
				return Parser::Parse(value, ref);
			};

			var.getter = [&ref]()
			{
				return Parser::ToString(ref);
			};

			return var;
		}
	};

	

	// =========================
	// Console Core
	// =========================
	class Console
	{
	public:
		// ---------- VARIABLES ----------
		static bool AddVariable(const Variable& var)
		{
			auto& vars = GetVariables();

			auto it = std::find_if(vars.begin(), vars.end(),
				[&](const Variable& v) { return v.name == var.name; });

			if (it != vars.end())
				return false;

			vars.push_back(var);
			return true;
		}

		static Variable* GetVariable(const std::string& name)
		{
			auto& vars = GetVariables();

			auto it = std::find_if(vars.begin(), vars.end(),
				[&](Variable& v) { return v.name == name; });

			return (it != vars.end()) ? &(*it) : nullptr;
		}

		static bool Set(const std::string& name, const std::string& value)
		{
			if (auto* var = GetVariable(name))
				return var->setter(value);

			return false;
		}

		static std::string Get(const std::string& name)
		{
			if (auto* var = GetVariable(name))
				return var->getter();

			return "null";
		}

		static std::vector<Variable>& GetVariables()
		{
			static std::vector<Variable> vars;
			return vars;
		}

		// ---------- COMMANDS ----------
		static bool AddCommand(const Command& cmd)
		{
			auto& cmds = GetCommands();

			auto it = std::find_if(cmds.begin(), cmds.end(),
				[&](const Command& c) { return c.name == cmd.name; });

			if (it != cmds.end())
				return false;

			cmds.push_back(cmd);
			return true;
		}

		static Command* GetCommand(const std::string& name)
		{
			auto& cmds = GetCommands();

			auto it = std::find_if(cmds.begin(), cmds.end(),
				[&](Command& c) { return c.name == name; });

			return (it != cmds.end()) ? &(*it) : nullptr;
		}

		static std::vector<Command>& GetCommands()
		{
			static std::vector<Command> cmds;
			return cmds;
		}

		// ---------- EXECUTION ----------
		static void Execute(const std::string& line)
		{
			auto tokens = Parser::Tokenize(line);
			if (tokens.empty()) return;

			const std::string& name = tokens[0];

			// 1. Commands
			if (auto* cmd = GetCommand(name))
			{
				tokens.erase(tokens.begin());
				cmd->func(tokens);
				return;
			}

			// 2. Variable set
			if (tokens.size() >= 2)
			{
				std::string value;
				for (size_t i = 1; i < tokens.size(); i++)
				{
					if (i > 1) value += " ";
					value += tokens[i];
				}

				if (Set(name, value))
				{
					BP_CORE_INFO("{0} = {1}", name, Get(name));
					return;
				}
			}

			// 3. Variable get
			if (auto* var = GetVariable(name))
			{
				BP_CORE_INFO("{0} = {1}", name, var->getter());
				return;
			}

			BP_CORE_ERROR("Unknown command or variable");
		}

		static std::vector<std::string> GetLogs() { return s_Logs; }
		static void AddLog(const std::string& log) { s_Logs.push_back(log); }

	private:
		static std::vector<std::string> s_Logs;
		
	};

	template<typename Mutex>
	class ConsoleSink : public spdlog::sinks::base_sink <Mutex>
	{
	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			spdlog::details::log_msg newMsg = spdlog::details::log_msg(msg);
			spdlog::memory_buf_t formatted;
			spdlog::sinks::base_sink<Mutex>::formatter_->format(newMsg, formatted);
			Console::AddLog(fmt::to_string(formatted));
			
		}
		void flush_() override
		{

		}
	};
}