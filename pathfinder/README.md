
Pathfinder uses a JSON string as input:


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
