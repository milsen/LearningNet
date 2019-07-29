#include <lemon/arg_parser.h>
#include <learningnet/NetworkChecker.hpp>
#include <learningnet/ActivitySetter.hpp>

using namespace lemon;
using namespace learningnet;

int main(int argc, char *argv[])
{
	ArgParser ap(argc, argv);
	std::string network;
	std::string sections;
	std::string costs;

	// Set option that can be parsed.
	ap
		// possible actions (one and only one must be used)
		.boolOption("check", "Check whether the input is a correct learning net. Param: network")
		.boolOption("create", "Create a learning net without edges. Param: sections")
		.boolOption("active", "Output active units based on completed ones. Param: network, sections")
		.boolOption("recnext", "Output the recommended next unit. Param: network, costs")
		.boolOption("recpath", "Output the sequence of recommended units. Param: network, costs")
		.optionGroup("action", "check")
		.optionGroup("action", "create")
		.optionGroup("action", "active")
		.optionGroup("action", "recnext")
		.optionGroup("action", "recpath")
		.mandatoryGroup("action")
		.onlyOneGroup("action")

		// possible arguments
		.refOption("network", "Network as space-separated string.", network)
		.refOption("costs", "Costs as space-separated string.", costs) // TODO will not work
		.refOption("sections", "Relevant sections as space-separated string.", sections)

		// whether costs are for nodes or edges
		.boolOption("edgecosts", "Use costs for edges.")
		.boolOption("nodecosts", "Use costs for nodes.")
		.optionGroup("costtype", "edgecosts")
		.optionGroup("costtype", "nodecosts")
		.onlyOneGroup("costtype");

	// Throw an exception when problems occurs.
	// The default behavior is to exit(1) on these cases, but this makes
	// Valgrind falsely warn about memory leaks.
	ap.throwOnProblems();

	// Perform the parsing process
	// (in case of any error it terminates the program)
	// The try {} construct is necessary only if the ap.trowOnProblems()
	// setting is in use.
	try {
		ap.parse();
	} catch (ArgParserException &e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	// Check for correct parameters.
	if ((ap.given("check")   && !ap.given("network"))
	 || (ap.given("create")  && !ap.given("sections"))
	 || (ap.given("active")  && !(ap.given("network") && ap.given("sections")))
	 || (ap.given("recnext") && !(ap.given("network") && ap.given("costs")))
	 || (ap.given("recpath") && !(ap.given("network") && ap.given("costs")))) {
		std::cout << "Not all necessary parameters for the given action found." << std::endl;
		return EXIT_FAILURE;
	}

	try {
		// Execute action.
		if (ap.given("check")) {
			LearningNet net(network);
			NetworkChecker checker(net);
			return checker.handleSuccess();
		} else if (ap.given("create")) {
			LearningNet *net = LearningNet::create(sections);
			net->write();
			delete net;
			return EXIT_SUCCESS;
		} else if (ap.given("active")) {
			LearningNet net(network);
			ActivitySetter act(net, sections);
			net.write();
			return act.handleSuccess();
		} else if (ap.given("recnext")) {
			return EXIT_FAILURE;
		} else if (ap.given("recpath")) {
			return EXIT_FAILURE;
		} else {
			std::cout << "No action given." << std::endl;
			return EXIT_FAILURE;
		}
	} catch (Exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
