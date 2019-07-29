#pragma once

namespace learningnet {

class Module
{
private:
	bool m_success;

	std::string m_error;

protected:
	Module() : m_success{true}, m_error{""} { }

	inline void appendError(const std::string &error) {
		if (!m_error.empty()) {
			m_error.append("\n");
		}
		m_error.append(error);
	}

	inline std::string getError() const {
		return m_error;
	}

	inline void failWithError(const std::string &error) {
		appendError(error);
		fail();
	}

	inline void fail() {
		m_success = false;
	}

public:
	inline bool succeeded() const {
		return m_success;
	}

	inline bool handleSuccess(std::ostream &out = std::cout) const {
		if (!m_success) {
			out << m_error;
		}
		return m_success;
	}
};

}
