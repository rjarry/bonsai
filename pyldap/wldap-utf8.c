#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include "wldap-utf8.h"

static char *
convert_to_mbs(wchar_t *tmp) {
	char *str = NULL;
	int size = 0;
	int rc = 0;

	if (tmp == NULL) return NULL;

	/* Get necessary size for the new UTF-8 encoded char*. */
	size = WideCharToMultiByte(CP_UTF8, 0, tmp, -1, NULL, 0, NULL, NULL);
	str = (char *)malloc(sizeof(char) * size);
	if (str == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	rc = WideCharToMultiByte(CP_UTF8, 0, tmp, -1, str, size, NULL, NULL);
	if (rc == 0) {
		free(str);
		PyErr_Format(PyExc_UnicodeError, "Converting to UTF-8 is failed.");
		return NULL;
	}

	return str;
}

static wchar_t *
convert_to_wcs(char *tmp) {
	wchar_t *str = NULL;
	int size = 0;
	int rc = 0;

	if (tmp == NULL) return NULL;

	/* Get necessary size for the new wchar_t*. */
	size = MultiByteToWideChar(CP_UTF8, 0, tmp, -1, NULL, 0);
	str = (wchar_t *)malloc(sizeof(wchar_t) * size);
	if (str == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	rc = MultiByteToWideChar(CP_UTF8, 0, tmp, -1, str, size);
	if (rc == 0) {
		free(str);
		PyErr_Format(PyExc_UnicodeError, "Converting from UTF-8 is failed.");
		return NULL;
	}

	return str;
}

static LDAPControlW *
convert_ctrl(LDAPControlA *ctrl) {
	wchar_t *woid = NULL;
	LDAPControlW *wctrl = NULL;

	wctrl = (LDAPControlW *)malloc(sizeof(LDAPControlW));
	if (wctrl == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	
	wctrl->ldctl_iscritical = ctrl->ldctl_iscritical;
	wctrl->ldctl_value = ctrl->ldctl_value;

	woid = convert_to_wcs(ctrl->ldctl_oid);
	wctrl->ldctl_oid = woid;

	return wctrl;
}

static void 
free_ctrl(LDAPControlW *ctrl) {
	if (ctrl != NULL) {
		if (ctrl->ldctl_oid) free(ctrl->ldctl_oid);
		free(ctrl);
	}
}

static LDAPControlW **
convert_ctrl_list(LDAPControlA **ctrls) {
	int size = 0;
	int i = 0;
	LDAPControlA *tmp = NULL;
	LDAPControlW **wctrls = NULL;

	/* Get the size of the list by reaching the terminating NULL. */
	while (ctrls[size] != NULL) size++;

	wctrls = (LDAPControlW **)malloc(sizeof(LDAPControlW *) * (size + 1));
	if (wctrls == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	for (tmp = ctrls[i]; tmp != NULL; i++) {
		wctrls[i] = convert_ctrl(tmp);
		if (wctrls[i] == NULL) return NULL;
	}
	wctrls[i] = NULL;

	return wctrls;
}

static void
free_ctrl_list(LDAPControlW **ctrls) {
	int i = 0;
	LDAPControlW *tmp = NULL;
	
	if (ctrls != NULL) {
		for (tmp = ctrls[i]; tmp != NULL; i++) {
			free_ctrl(tmp);
		}
	}
}

static LDAPModW *
convert_mod(LDAPModA *mod) {
	wchar_t *wtype = NULL;
	wchar_t **list = NULL;
	LDAPModW *wmod = NULL;

	wmod = (LDAPModW *)malloc(sizeof(LDAPModW));
	if (wmod == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	wmod->mod_op = mod->mod_op;
	wmod->mod_vals.modv_bvals = mod->mod_vals.modv_bvals;
	
	list = convert_char_list(mod->mod_vals.modv_strvals);
	wmod->mod_vals.modv_strvals = list;

	wtype = convert_to_wcs(mod->mod_type);
	wmod->mod_type = wtype;

	return wmod;
}

static void
free_mod(LDAPModW *mod) {
	if (mod != NULL) {
		if (mod->mod_type) free(mod->mod_type);
		if (mod->mod_vals.modv_strvals) free_char_list(mod->mod_vals.modv_strvals);
		free(mod);
	}
}

static LDAPModW **
convert_mod_list(LDAPModA **mods) {
	int size = 0;
	int i = 0;
	LDAPModA *tmp = NULL;
	LDAPModW **wmods = NULL;

	/* Get the size of the list by reaching the terminating NULL. */
	while (mods[size] != NULL) size++;

	wmods = (LDAPModW **)malloc(sizeof(LDAPModW *) * (size + 1));
	if (wmods == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	for (tmp = mods[i]; tmp != NULL; i++) {
		wmods[i] = convert_mod(tmp);
		if (wmods[i] == NULL) return NULL;
	}
	wmods[i] = NULL;

	return wmods;
}

static void
free_mod_list(LDAPModW **mods) {
	int i = 0;
	LDAPModW *tmp = NULL;

	if (mods != NULL) {
		for (tmp = mods[i]; tmp != NULL; i++) {
			free_mod(tmp);
		}
	}
}

static wchar_t **
convert_char_list(char **list) {
	int size = 0;
	int i = 0;
	char *tmp = NULL;
	wchar_t **wlist = NULL;

	/* Get the size of the list by reaching the terminating NULL. */
	while (list[size] != NULL) size++;

	wlist = (wchar_t **)malloc(sizeof(wchar_t *) * (size + 1));
	if (wlist == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	for (tmp = list[i]; tmp != NULL; i++) {
		wlist[i] = convert_ctrl(tmp);
		if (wlist[i] == NULL) return NULL;
	}
	wlist[i] = NULL;

	return wlist;
}

static void
free_char_list(wchar_t **list) {
	int i = 0;
	wchar_t *tmp = NULL;

	if (list != NULL) {
		for (tmp = list[i]; tmp != NULL; i++) {
			free(tmp);
		}
	}
}

static LDAPSortKeyW *
convert_sortkey(LDAPSortKeyA *sortkey) {
	wchar_t *attrtype = NULL;
	wchar_t *ruleoid = NULL;
	LDAPSortKeyW *wsortkey = NULL;

	wsortkey = (LDAPSortKeyW *)malloc(sizeof(LDAPSortKeyW));
	if (wsortkey == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	attrtype = convert_to_wcs(sortkey->sk_attrtype);
	wsortkey->sk_attrtype = attrtype;
	
	ruleoid = convert_to_wcs(sortkey->sk_matchruleoid);
	wsortkey->sk_matchruleoid = ruleoid;

	wsortkey->sk_reverseorder = sortkey->sk_reverseorder;

	return wsortkey;
}

static void
free_sortkey(LDAPSortKeyW *sortkey) {
	if (sortkey != NULL) {
		if (sortkey->sk_attrtype) free(sortkey->sk_attrtype);
		if (sortkey->sk_matchruleoid) free(sortkey->sk_matchruleoid);
	}
}

static LDAPSortKeyW **
convert_sortkey_list(LDAPSortKeyA **keylist) {
	int size = 0;
	int i = 0;
	LDAPSortKeyA *tmp = NULL;
	LDAPSortKeyW **wkeylist = NULL;

	/* Get the size of the list by reaching the terminating NULL. */
	while (keylist[size] != NULL) size++;

	wkeylist = (wchar_t **)malloc(sizeof(LDAPSortKeyW *) * (size + 1));
	if (wkeylist == NULL) {
		PyErr_NoMemory();
		return NULL;
	}

	for (tmp = keylist[i]; tmp != NULL; i++) {
		wkeylist[i] = convert_sortkey(tmp);
		if (wkeylist[i] == NULL) return NULL;
	}
	wkeylist[i] = NULL;

	return wkeylist;
}

static void
free_sortkey_list(LDAPSortKeyW **keylist) {
	int i = 0;
	LDAPSortKeyW *tmp = NULL;

	if (keylist != NULL) {
		for (tmp = keylist[i]; tmp != NULL; i++) {
			free_sortkey(tmp);
		}
	}
}

int
ldap_unbind_ext(LDAP *ld, LDAPControl **sctrls, LDAPControl	**cctrls) {
	return ldap_unbind(ld);
}

int
ldap_abandon_ext(LDAP *ld, int msgid, LDAPControl **sctrls, LDAPControl	**cctrls) {
	return ldap_abandon(ld, msgid);
}

char *
ldap_get_dnU(LDAP *ld, LDAPMessage *entry) {
	char *dn = NULL;
	wchar_t *wdn = NULL;

	wdn = ldap_get_dnW(ld, entry);

	dn = convert_to_mbs(wdn);

	ldap_memfreeW(wdn);

	return dn;
}

int
ldap_add_extU(LDAP *ld, char *dn, LDAPMod **attrs, LDAPControl **sctrls, LDAPControl **cctrls,
		int *msgidp) {
	
	int rc = 0;
	wchar_t wdn = NULL;
	LDAPModW **wattrs = NULL;
	LDAPControlW **wsctrls = NULL;
	LDAPControlW **wcctrls = NULL;

	wsctrls = convert_ctrl_list(sctrls);
	wcctrls = convert_ctrl_list(cctrls);
	wattrs = convert_mod_list(attrs);

	rc = ldap_add_extW(ld, wdn, wattrs, wsctrls, wcctrls, msgidp);

	free_ctrl_list(wsctrls);
	free_ctrl_list(wcctrls);
	free_mod_list(wattrs);

	return rc;
}

int
ldap_modify_extU(LDAP *ld, char *dn, LDAPMod **attrs, LDAPControl **sctrls, LDAPControl **cctrls,
		int *msgidp) {
	
	int rc = 0;
	wchar_t wdn = NULL;
	LDAPModW **wattrs = NULL;
	LDAPControlW **wsctrls = NULL;
	LDAPControlW **wcctrls = NULL;

	wsctrls = convert_ctrl_list(sctrls);
	wcctrls = convert_ctrl_list(cctrls);
	wattrs = convert_mod_list(attrs);

	rc = ldap_modify_extW(ld, wdn, wattrs, wsctrls, wcctrls, msgidp);

	free_ctrl_list(wsctrls);
	free_ctrl_list(wcctrls);
	free_mod_list(wattrs);

	return rc;
}

char *
ldap_first_attributeU(LDAP *ld, LDAPMessage *entry, BerElement **ber) {
	char *attr = NULL;
	wchar_t wattr = NULL;

	wattr = ldap_first_attributeW(ld, entry, ber);

	attr = convert_to_mbs(wattr);

	ldap_memfreeW(wattr);

	return attr;
}

char *
ldap_next_attributeU(LDAP *ld, LDAPMessage *entry, BerElement *ber) {
	char *attr = NULL;
	wchar_t wattr = NULL;

	wattr = ldap_next_attributeW(ld, entry, ber);

	attr = convert_to_mbs(wattr);

	ldap_memfreeW(wattr);

	return attr;
}

struct berval **
ldap_get_values_lenU(LDAP *ld, LDAPMessage *entry, char *target) {
	struct berval **ret = NULL;
	wchar_t *wtarget = NULL;

	wtarget = convert_to_wcs(target);

	ret = ldap_get_values_lenW(ld, entry, wtarget);

	if (wtarget) free(wtarget);

	return ret;
}

int
ldap_renameU(LDAP *ld, char *dn, char *newrdn, char *newSuperior, int deleteoldrdn,
		LDAPControl **sctrls, LDAPControl **cctrls, int *msgidp) {
	
	int rc = 0;
	wchar_t *wdn = NULL; 
	wchar_t *wnewrdn = NULL;
	wchar_t *wnewSuperior = NULL;
	LDAPControlW **wsctrls = NULL;
	LDAPControlW **wcctrls = NULL;
	
	wdn = convert_to_wcs(dn);
	wnewrdn = convert_to_wcs(newrdn);
	wnewSuperior = convert_to_wcs(newSuperior);
	wsctrls = convert_ctrl_list(sctrls);
	wcctrls = convert_ctrl_list(cctrls);

	rc = ldap_rename_extW(ld, wdn, wnewrdn, wnewSuperior, deleteoldrdn, wsctrls, wcctrls, msgidp);

	if (wdn) free(wdn);
	if (wnewrdn) free(wnewrdn);
	if (wnewSuperior) free(wnewSuperior);
	free_ctrl_list(wsctrls);
	free_ctrl_list(wcctrls);

	return rc;
}

int
ldap_search_extU(LDAP *ld, char *base, int scope, char *filter, char **attrs, int attrsonly,
		LDAPControl **sctrls, LDAPControl **cctrls, struct timeval *timeout, int sizelimit, int *msgidp) {
	
	int rc = 0;
	wchar_t *wbase = NULL;
	wchar_t *wfilter = NULL;
	wchar_t **wattrs = NULL;
	LDAPControlW **wsctrls = NULL;
	LDAPControlW **wcctrls = NULL;
	unsigned long timelimit = 0;

	if (timeout != NULL) {
		timelimit = (unsigned long)timeout->tv_sec;
	} else {
		timelimit = 0;
	}

	wbase = convert_to_wcs(base);
	wfilter = convert_to_wcs(filter);
	wattrs = convert_char_list(attrs);
	wsctrls = convert_ctrl_list(sctrls);
	wcctrls = convert_ctrl_list(cctrls);

	rc = ldap_search_extW(ld, wbase, scope, wfilter, wattrs, attrsonly, wsctrls, wcctrls, timelimit,
			sizelimit, msgidp);

	if (wbase) free(wbase);
	if (wfilter) free(wfilter);
	free_char_list(wattrs);
	free_ctrl_list(wsctrls);
	free_ctrl_list(wcctrls);

	return rc;
}

int
ldap_create_sort_controlU(LDAP *ld, LDAPSortKey **keyList, int iscritical, LDAPControl **ctrlp) {
	int rc = 0;
	LDAPSortKeyW **wkeylist = NULL;
	LDAPControlW *wctrlp = NULL;
	LDAPControlA *ret = NULL;

	wkeylist = convert_sortkey_list(keyList);

	rc = ldap_create_sort_controlW(ld, wkeylist, iscritical, &wctrlp);

	free_sortkey_list(wkeylist);

	ret = (LDAPControlA *)malloc(sizeof(LDAPControlA));
	if (ret == NULL) {
		PyErr_NoMemory();
		return -1;
	}

	ret->ldctl_iscritical = wctrlp->ldctl_iscritical;
	ret->ldctl_oid = convert_to_mbs(wctrlp->ldctl_oid);
	ret->ldctl_value = wctrlp->ldctl_value;

	*ctrlp = ret;

	ldap_control_freeW(wctrlp);

	return rc;
}

int
ldap_extended_operationU(LDAP *ld, char *reqoid, struct berval *reqdata, LDAPControl **sctrls,
		LDAPControl **cctrls, int *msgidp) {
	
	int rc = 0;
	wchar_t *woid = NULL;
	LDAPControlW **wsctrls = NULL;
	LDAPControlW **wcctrls = NULL;

	woid = convert_to_wcs(reqoid);
	wsctrls = convert_ctrl_list(sctrls);
	wcctrls = convert_ctrl_list(cctrls);

	rc = ldap_extended_operationW(ld, woid, reqdata, wsctrls, wcctrls, msgidp);

	if (woid) free(woid);
	free_ctrl_list(wsctrls);
	free_ctrl_list(wcctrls);

	return rc;
}

int
ldap_parse_extended_resultU(LDAP *ld, LDAPMessage *res, char **retoidp, struct berval **retdatap,
		int freeit) {
	
	int rc = 0;
	char *oid = NULL;
	wchar_t *wretoid = NULL;

	rc = ldap_parse_extended_resultW(ld, res, &wretoid, retdatap, freeit);

	oid = convert_to_mbs(wretoid);

	if (wretoid) ldap_memfreeW(wretoid);

	*retoidp = oid;
	return rc;
}

int
ldap_parse_pageresponse_controlU(LDAP *ld, LDAPControl *ctrl, ber_int_t *count,
		struct berval *cookie) {
	
	int rc = 0;
	LDAPControlW *wctrl = NULL;
	LDAPControlW **list = NULL;

	wctrl = convert_ctrl(ctrl);

	/* Create a NULL terminated list of controls including the page control. */
	list = (LDAPControlW **)malloc(sizeof(LDAPControlW) * 2);
	if (list == NULL) {
		PyErr_NoMemory();
		return -1;
	}
	list[0] = wctrl;
	list[1] = NULL;

	if (cookie != NULL && cookie->bv_val != NULL) {
		/* Clear the cookie's content for the new data. */
		ber_bvfree(cookie);
		cookie = NULL;
	}

	rc = ldap_parse_page_controlW(ld, list, (unsigned long *)count, &cookie);

	free_ctrl(wctrl);
	free(list);

	return rc;
}

LDAPControl *
ldap_control_findU(char *oid, LDAPControl **ctrls, LDAPControl ***nextctrlp) {
	/* This function is a copy from the OpenLDAP's controls.c source file. */
	if (oid == NULL || ctrls == NULL || *ctrls == NULL) {
		return NULL;
	}

	for (; *ctrls != NULL; ctrls++) {
		if (strcmp((*ctrls)->ldctl_oid, oid) == 0) {
			if (nextctrlp != NULL) {
				*nextctrlp = ctrls + 1;
			}
			return *ctrls;
		}
	}

	if (nextctrlp != NULL) {
		*nextctrlp = NULL;
	}

	return NULL;
}

int
ldap_parse_resultU(LDAP *ld, LDAPMessage *res, int *errcodep, char **matcheddnp, char **errmsgp,
		char ***referralsp, LDAPControl ***sctrls, int freeit) {
	
	int i = 0;
	int rc = 0;
	int size = 0;
	char **refs = NULL;
	wchar_t *tmp = NULL;
	wchar_t *wmatcheddnp = NULL;
	wchar_t *werrmsgp = NULL;
	wchar_t **wreferralsp = NULL;
	LDAPControlW **wsctrls = NULL;
	LDAPControlA **ctrls = NULL;
	LDAPControlA *ctrla = NULL;
	LDAPControlW *ctmp = NULL;

	rc = ldap_parse_resultW(ld, res, errcodep, &wmatcheddnp, &werrmsgp, &wreferralsp, &wsctrls, freeit);

	*matcheddnp = convert_to_mbs(wmatcheddnp);
	*errmsgp = convert_to_mbs(werrmsgp);

	/* Copy and convert the refreal strings. */
	while (wreferralsp[size] != NULL) size++;

	refs = (char **)malloc(sizeof(char *) * (size + 1));
	if (refs == NULL) {
		PyErr_NoMemory();
		return -1;
	}

	for (tmp = wreferralsp[i]; tmp != NULL; i++) {
		refs[i] = convert_to_mbs(tmp);
	}
	refs[i] = NULL;

	*referralsp = refs;

	/* Copy and convert the server controls. */
	size = 0;
	while (wsctrls[size] != NULL) size++;
	ctrls = (LDAPControlA **)malloc(sizeof(LDAPControlA *) * (size + 1));
	if (ctrls == NULL) {
		PyErr_NoMemory();
		return -1;
	}

	for (ctmp = wsctrls[i]; ctmp != NULL; i++) {
		ctrla = (LDAPControlA *)malloc(sizeof(LDAPControlA));
		if (ctrla == NULL) {
			PyErr_NoMemory();
			return -1;
		}
		ctrla->ldctl_iscritical = ctmp->ldctl_iscritical;
		ctrla->ldctl_oid = convert_to_mbs(ctmp->ldctl_oid);
		ctrla->ldctl_value = *ber_bvdup(&(ctmp->ldctl_value));
		ctrls[i] = ctrla;

	}
	ctrls[i] = NULL;

	*sctrls = ctrls;

	ldap_memfreeW(wmatcheddnp);
	ldap_memfreeW(werrmsgp);
	ldap_value_freeW(wreferralsp);
	ldap_controls_freeW(wsctrls);

	return rc;
}

char *
ldap_err2stringU(int err) {
	wchar_t *werr = NULL;

	werr = ldap_err2stringW(err);

	return convert_to_mbs(werr);
}

int
ldap_simple_bind_sU(LDAP *ld, char *who, char *passwd) {
	int rc = 0;
	wchar_t *wwho = NULL;
	wchar_t *wpsw = NULL;

	wwho = convert_to_wcs(who);
	wpsw = convert_to_wcs(passwd);

	rc = ldap_simple_bind_sW(ld, wwho, wpsw);

	if (wwho) free(wwho);
	if (wpsw) free(wpsw);

	return rc;
}

int
ldap_sasl_interactive_bind_sU(LDAP *ld, char *dn, char *mechanism, LDAPControl **sctrls,
		LDAPControl **cctrls, unsigned flags, LDAP_SASL_INTERACT_PROC *proc, void *defaults) {
	
	int rc = 0;
	wchar_t *wdn = NULL;
	wchar_t *wmech = NULL;
	struct berval cred;
	struct berval *response = NULL;
	LDAPControlW **wsctrls = NULL;
	LDAPControlW **wcctrls = NULL;

	wdn = convert_to_wcs(dn);
	wmech = convert_to_wcs(mechanism);
	wsctrls = convert_ctrl_list(sctrls);
	wcctrls = convert_ctrl_list(cctrls);

	do {
		rc = (proc)(ld, (sasl_defaults_t *)defaults, response, &cred);
		
		/* NULL binddn is needed to change empty string to avoid param error. */
		if (wdn == NULL) wdn = L"";
		rc = ldap_sasl_bind_sW(ld, wdn, wmech, &cred, wsctrls, wcctrls, response);
		
		/* Get the last error code from the LDAP struct. */
		ldap_get_option(ld, LDAP_OPT_ERROR_NUMBER, &rc);
	} while (rc == LDAP_SASL_BIND_IN_PROGRESS);
	
	if (wdn) free(wdn);
	if (wmech) free(wmech);
	free_ctrl_list(wsctrls);
	free_ctrl_list(wcctrls);

	return rc;
}

#endif