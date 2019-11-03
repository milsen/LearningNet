#include <string>
#include <iostream>

#pragma once

namespace learningnet {

class Module
{
private:
	bool m_failed;

	std::string m_error;

protected:
	Module() : m_failed{false}, m_error{""} { }

	inline void appendError(const std::string &error) {
		if (!m_error.empty()) {
			m_error.append("\n");
		}
		m_error.append(error);
	}

	inline void failWithError(const std::string &error) {
		appendError(error);
		fail();
	}

	inline void fail() {
		m_failed = true;
	}

public:
	inline bool succeeded() const {
		return !m_failed;
	}

	inline std::string getError() const {
		return m_error;
	}

	inline bool handleFailure(std::ostream &out = std::cout) const {
		if (m_failed) {
			out << m_error;
		}
		return m_failed ? EXIT_FAILURE : EXIT_SUCCESS;
	}
};

}
