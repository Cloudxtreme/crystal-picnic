package com.nooskewl.crystalpicnic;

import java.util.HashSet;
import java.util.Set;

public enum MySKU {

	UNLOCK("UNLOCK");

	private String sku;

	private MySKU(String sku) {
		this.sku = sku;
	}

	public static MySKU valueForSKU(String sku) {
		if (UNLOCK.getSku().equals(sku)) {
			return UNLOCK;
		}
		return null;
	}

	public String getSku() {
		return sku;
	}

	private static Set<String> SKUS = new HashSet<String>();
	static {
		SKUS.add(UNLOCK.getSku());
	}

	public static Set<String> getAll() {
		return SKUS;
	}

}
