#include <string>
#include <iostream>

#pragma once

namespace learningnet {

/**
 * Superclass for classes that encapsulate an algorithm.
 * Stores whether the algorithm has failed and a corresponding error message.
 */
class Module
{
private:
	bool m_failed; //!< Whether the procedure has failed.

	std::string m_error; //!< Why the procedure has failed.

protected:
	/**
	 * Creates a new Module that has not failed yet.
	 */
	Module() : m_failed{false}, m_error{""} { }

	/**
	 * Appends an error to the error string of this Module.
	 *
	 * @param error the string to append
	 */
	inline void appendError(const std::string &error) {
		if (!m_error.empty()) {
			m_error.append("\n");
		}
		m_error.append(error);
	}

	/**
	 * Appends an error to the error string of this Module and marks it as
	 * having failed.
	 *
	 * @param error the string to append
	 */
	inline void failWithError(const std::string &error) {
		appendError(error);
		fail();
	}

	/**
	 * Marks this Module as having failed.
	 */
	inline void fail() {
		m_failed = true;
	}

public:
	/**
	 * @return whether this Module has not failed yet
	 */
	inline bool succeeded() const {
		return !m_failed;
	}

	/**
	 * @return the error string of this Module
	 */
	inline std::string getError() const {
		return m_error;
	}

	/**
	 * Writes the error string of this Module to a given stream and return
	 * EXIT_FAILURE or EXIT_SUCCESS depending on whether this Module has failed.
	 *
	 * @param out the stream to which the error message is written
	 * @return EXIT_FAILURE if the Module has failed, EXIT_SUCCESS otherwise
	 */
	inline bool handleFailure(std::ostream &out = std::cout) const {
		if (m_failed) {
			out << m_error;
		}
		return m_failed ? EXIT_FAILURE : EXIT_SUCCESS;
	}
};

}
