document.addEventListener("DOMContentLoaded", () => {
	document.querySelectorAll(".wy-menu-vertical > p.caption").forEach((caption) => {
		const list = caption.nextElementSibling;
		if (!list || list.tagName !== "UL")
			return;

		const details = document.createElement("details");
		details.open = Boolean(list.querySelector(".current"));

		const summary = document.createElement("summary");
		summary.className = caption.className;
		summary.append(...caption.childNodes);
		details.append(summary, list);
		caption.replaceWith(details);
	});
});
