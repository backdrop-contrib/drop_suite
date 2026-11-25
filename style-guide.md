# Drop Suite Style Guide

Design decisions to remember across the Drop Suite modules.

## Theming Strategy

Drop Suite is **theme-agnostic**. All styling lives in the modules themselves,
not in a dedicated theme.

- **Shared styles** go in `drop_suite.css` — nav, priority colors, section headers,
  page actions, settings cards, CSS custom properties. This file is loaded on
  every page where drop_suite is active.
- **Module-specific styles** go in each module's own CSS file (e.g.
  `droptickets.css`, `drop_crm.admin.css`) and are loaded via `#attached` only
  on that module's pages.
- **No Drop Suite theme.** The Dropper theme was an early experiment (March
  2026, forked from Tatsu) and is no longer used or developed. Drop Suite looks
  better in established themes that have solid typography, form styling, and
  table defaults out of the box.

**Where to put new CSS:**
- If it applies to multiple modules (e.g. a new utility class, a shared
  component) → `drop_suite.css`.
- If it only applies to one module's pages → that module's CSS file.
- Never in a theme. Drop Suite should work with any well-built Backdrop theme.

### Recommended theme: Opera (never required)

- **Opera 2.x with the Design Tokens module is the recommended and tested
  theme.** Drop Start recommends it during setup, and every Drop Suite dev site
  runs it.
- **Drop Suite must not depend on Opera or on Design Tokens.** Every page must
  work and look reasonable on any well-built Backdrop theme. Never target
  Opera's class names or its `--color-*` variables in Drop Suite CSS.
- **When a page looks wrong in Opera**, first decide whether the fix belongs in
  Drop Suite (our markup or CSS) or in Opera (a general theme bug). Opera fixes
  go to the Opera repo, not into Drop Suite CSS.
- **Colours come from Drop Suite's own palette**, not from the theme (see
  *Use of Color*). With Design Tokens enabled, Site Architects can change the
  palette on the theme's Design Tokens page, in the "Drop Suite" group.

## Density and use of space

Drop Suite is a working tool — people live inside these forms and tables
every day. Design for **scannability and density**, not for marketing
polish. Default theme padding is tuned for content pages and looks
loose and wasteful inside a working UI.

**Apply everywhere — forms, tables, cards, sidebars, list pages:**

- **Make good use of horizontal real estate.** Put short related fields
  side-by-side (e.g. start date + end date, first name + last name,
  label + machine key). Don't stack every field in a full-width column
  just because it's the default — that forces excessive scrolling and
  wastes the right half of the page.
- **Tighten vertical rhythm.** Reduce default margin/padding on form-items
  and table cells embedded in admin surfaces. Aim for rows that read
  as one continuous list, not as isolated blocks.
- **Cap field widths to fit their content.** A 24-char label doesn't
  need a 60-char input. Use `#size` on textfields and `max-width` on
  the rendered input to prevent them from stretching across the page.
- **Columns should be only as wide as they need to be.** Give narrow
  columns (checkboxes, icons, short keys) explicit widths so the
  browser doesn't stretch them to fill available space.
- **Status / meta labels render inline**, not stacked below the element
  they annotate. "In use", "Required", "Read-only" all belong next to
  the control, not under it.
- **Stay within the page's max-width.** Narrow pages (settings, entity
  views) use the configured narrow width — don't break out of it for
  a single wide table. Tighten the content instead.

When in doubt, compare against the Opera theme's default form and ask:
*"could this fit twice as much information on the screen without
feeling cramped?"* If yes, tighten it. If no, leave it.

**Reference example:** the primary-navigation admin form at
`/drop/settings/navigation` is a good worked example of these principles —
a drag-reorder table with inline fields (label, icon, path, hide, delete)
packed into one tight row per nav item, narrow column widths for the
checkbox and icon fields, zero margin on form-items, and a short bullet
list above the table instead of a prose intro. When building a similar
management UI, open that page first and mirror its density.
See `drop_suite.nav_admin.inc` in this module.

## Navigation

Four layered surfaces. Order from site-wide to page-specific:

1. **Primary nav** — site-wide sections (Dashboard, CRM, Tickets, Time…).
2. **Section nav** — sub-items within the current primary section (People, Organizations, To Do…).
3. **Local Tasks** — alternate views of the current page (View / Edit / Delete).
4. **Action Bar** — verbs acting on the current entity (Renew, Record payment, Cancel…). See *Action Bar* section below.

Each surface sits directly below the next broader one in the layout (primary → section → tabs → action bar → page title → content).

**Breadcrumbs are not a navigation surface.** The Drop Suite layout has no
breadcrumb block. Entity forms and pages still set a breadcrumb with
`backdrop_set_breadcrumb()` (see *Entity Edit Forms*), so sites that place a
breadcrumb block, or use another layout, get a sensible trail.

**Primary nav** (`drop-nav` block from drop_suite):
- Modules register links via `hook_drop_nav_links()` returning `title`, `path`,
  `permission`, `weight`, and optional `separator`.
- Right-side extras (e.g. notification bell) via `hook_drop_nav_extras()`.
- Rendered as `<nav class="drop-nav">` with flex layout, centered anchors.
- Active link gets `drop-nav-active` class.

**Section nav** (`drop_section_nav` block from drop_suite):
- Per-module sub-navigation based on the second URL segment (e.g. `drop/tickets/*`).
- Modules register links via `hook_drop_section_links($section)` returning
  `title`, `href`, `weight`, optional `badge`, `admin_only`, `active_paths`, `pinned`.
- Badges render as `<span class="drop-snav__badge">` — used for counts (flagged,
  unread, etc.).
- Pinned reports automatically appear in section nav at weight 50+.
- Active links get bottom border; admin-only links render smaller and gray.

**Local Tasks** (`MENU_LOCAL_TASK`):
- Use for standard Backdrop **View / Edit / Delete** tabs on entity view pages, and for admin sub-pages (Settings, Field UI, etc.) under a section root.
- A local task is an alternate **view** of the current thing — a different rendering of the same subject.
- Do NOT use `MENU_LOCAL_TASK` for verbs or actions (Renew, Cancel, Record payment, Send email, Extend due date). Those go in the **Action Bar**.
- The tabs bar renders directly below Section nav, above the page title. This positioning is seeded by `drop_suite_install` and maintained by `drop_suite_update_1204`.
- Use Backdrop's conventions. Content editors coming from other Backdrop sites should recognise the View/Edit tab pattern immediately.

**Local Actions** (`MENU_LOCAL_ACTION`):
- Use for user-facing "Add new X" links on listing pages (e.g. `+ Add Person`
  on the People view). These render as action links below the page title.

**Admin path nesting.** When a module presents itself as a sub-section of a
parent module in section nav, its admin paths should live under the parent's
URL namespace (e.g. `drop/crm/events/*` for DropEvents since Events appears
in CRM's section sub-nav; `drop/crm/memberships/*` for drop_crm_membership as
a submodule of Drop CRM). This keeps section detection aligned with the user's
mental model — the parent submenu persists across sub-section pages
automatically, no special-casing needed in `_drop_suite_get_section()`.
Public-facing routes for the same module are exempt and go wherever they read
best (e.g. `/events/{title}` for public event nodes).

## Action Bar

Per-entity verbs — things you *do to* the entity, not alternate views of it. Examples: Renew a membership, Record a payment, Send an email, Extend a due date, Cancel an order.

**Rendering** — call `drop_suite_page_actions(array $links)` near the top of the entity view page's render output. Returns a `<div class="drop-page-actions">` containing pill-styled links. Placed at the very start of `$output`, above the first section heading.

```php
$actions = array();
if (user_access('manage drop_crm_membership')) {
  $actions[] = l(t('Renew'), 'drop/crm/memberships/' . $membership->mid . '/renew');
  $actions[] = l(t('Record payment'), 'drop/crm/memberships/' . $membership->mid . '/payment/add');
}
if (user_access('cancel drop_crm_membership')) {
  $actions[] = l(t('Cancel'), 'drop/crm/memberships/' . $membership->mid . '/cancel');
}
$output['actions'] = array(
  '#markup' => drop_suite_page_actions($actions),
  '#weight' => -100,
);
```

**Routing** — the underlying paths (`/renew`, `/cancel`, etc.) are still menu items, but declared as `MENU_CALLBACK` (no tab, no action link) since they're invoked exclusively from the action bar. Keeping them routable lets modules cross-link to specific actions.

**Variants:**
- **Person contacts in Drop CRM** embed the action bar inside a richer "hero" block (`_drop_crm_contact_hero`) that also renders name, rating, tags, and job title. That's a decorative wrapper — the action bar below the hero still uses the same pill styling.
- **Org / household contacts** use a plain `drop_suite_page_actions()` call with Edit and Delete links.

**When NOT to use the action bar:**
- Alternate views of the entity (→ Local Tasks).
- "Add new X" on a listing page (→ Local Actions).
- Site-wide or section-level navigation (→ Primary nav / Section nav).

**Existing usage:** Drop CRM (org/household contact view pages), DropTasks (list page), DropGoals (history page), DropTickets (ticket list shortcut buttons). New entity view pages should follow the same pattern.

## Tables

- Build tables with `'#theme' => 'table'` render arrays, not raw HTML.
- Striped rows via `tbody tr:nth-child(even)` with light background (`#f7f8fa`).
- Priority row classes from drop_suite: `tr.drop-priority-critical` (red tint),
  `tr.drop-row-overdue` (red), `tr.drop-row-due-soon` (orange).
- Priority cell classes: `td.drop-priority-critical` through `td.drop-priority-none`
  with centered bold text and colored backgrounds.
- Reviewed/done rows: `tr.droptickets-review-row--done` with `opacity: 0.3` and
  strikethrough.
- Hot/warm status rows (CRM clients): light red (`#fef2f2`) and light yellow
  (`#fffbeb`) backgrounds.

### Compact admin tables

Tables embedded inside settings forms (activity types, status editor,
matching rules, etc.) must be **compact** — themes' default cell padding
is sized for content pages and looks loose inside a fieldset. Standard
compact styling:

- `border-collapse: collapse; font-size: 0.9em;`
- Cell padding `0.25rem 0.6rem` (about a third of the theme default).
- Zero margin on `.form-item` inside cells (`margin: 0; padding: 0;`)
  so inline form fields don't inflate row height.
- Explicit column widths on narrow columns (checkbox columns ~6rem,
  key/ID columns ~8rem) — prevents the browser from stretching them
  to eat wide empty space.
- Status labels (e.g. "In use" for rows that can't be deleted) render
  inline as muted italic text, not as a `#description` under a disabled
  checkbox — descriptions stack vertically and blow the row height up.

## Filters

GET-based filter forms with instant submit on select change.

**Standard pattern:**
```html
<form method="get" action="..." class="droptickets-filter-form">
  <select name="status" onchange="this.form.submit()">...</select>
  <select name="pid" onchange="this.form.submit()">...</select>
  <input type="search" name="search" placeholder="Search...">
  <button type="submit">...</button>
</form>
```

- Wrap in `<div class="droptickets-filter-row">` alongside mode buttons and badges.
- Status dropdown uses `<optgroup>` to organize: default "Active" option, then
  "By status" group, then "Other" group (All tickets, Imported, Discussions).
- Default select option uses em-dash format: `— All projects —`.
- Active tag filters show as a dismissible badge with `×` to clear.
- Hidden inputs to preserve persistent filters (e.g. tag filter kept across
  status changes).
- Shared helper `_droptickets_status_select_html($selected)` builds the status
  dropdown for pages that share the same filter set (Review, Recent, Stale).

## Landing Pages for Modules

Each module's main page follows a consistent structure:

1. **Page title** via `backdrop_set_title()`.
2. **Attached CSS** for the module.
3. **Filter bar** (if applicable) — status, project, search, toggles.
4. **Project Mode button** (tickets) — enter/exit to freeze filters with a
   persistent back-link on each item.
5. **Content area** — table, list, or summary depending on the module.

The section nav block (placed in the layout) provides sub-page links automatically.

| Module      | Landing path    | Content style                          |
|-------------|-----------------|----------------------------------------|
| droptickets | drop/tickets    | Filter bar + paginated table (25/page) |
| drop_crm     | drop/crm        | Summary counts + outreach goal widget  |
| droptasks   | drop/tasks      | Filter tabs + themed task list         |
| droptime    | drop/time       | Date nav + timer + daily entry list    |
| dropgoals   | drop/goals      | Goal cards with completion rates       |
| drop_docs    | drop/docs       | Document list                          |

## Page Width

- **Default max-width**: 1600px via `.l-wrapper-inner` in the Drop Suite layout.
- **Narrow pages**: Entity view pages (ticket, task, contact, goal, doc, settings)
  get the `drop-narrow-page` class, which applies a configurable max-width
  (set at `drop/settings/base`). This keeps reading-focused pages comfortable.
  Detected by `_drop_suite_is_narrow_page()`.
- **Kanban-wide mode**: The kanban board gets `layout-kanban-wide` class, expanding
  to `max-width: 95vw` so columns have room to spread.
- **Header region**: The `.l-header-inner` wrapper follows the Backdrop core layout
  convention and carries `container container-fluid` classes, matching moscone,
  boxton, and the other core layouts. This ensures the header sits inside the
  theme's standard container width rather than escaping it when the drop-nav
  block uses its full-bleed negative-margin trick. Don't remove these classes.

## Use of Color

All colours are CSS custom properties defined in the `:root` block of
`drop_suite.css`. That file is the single source of truth, so Drop Suite has
the same colours on every theme.

**Base palette** (defaults; also listed in `drop_suite_palette()`, keep the two
in sync):

| Property | Default | Used for |
|---|---|---|
| `--drop-primary` | #1565c0 | Links, primary buttons, active items |
| `--drop-primary-dark` | #0d47a1 | Hover state of primary |
| `--drop-text` | #222222 | Body text in Drop Suite components |
| `--drop-text-secondary` | #555555 | Secondary text, labels |
| `--drop-text-muted` | #888888 | Meta text, hints, empty states |
| `--drop-border` | #e0e0e0 | Borders and dividers |
| `--drop-bg-subtle` | #f5f5f5 | Subtle panels, striped rows |
| `--drop-bg-hover` | #f0f0f0 | Row and item hover |
| `--drop-highlight` | #e3f2fd | Selected or highlighted items |
| `--drop-success` | #2e7d32 | Completed, success states |
| `--drop-warning` | #b26a00 | Warnings and attention flags |
| `--drop-danger` | #c62828 | Errors, destructive actions |

**Design Tokens.** When the Design Tokens module is enabled, the palette
appears as a "Drop Suite" group on the theme's Design Tokens page (added by
`drop_suite_design_tokens_info_alter()`). Saved values override the defaults.
Drop Suite colours are **not** mapped onto a theme's own tokens: Opera's
`--color-primary`, for example, is its header background, not an accent colour.

**Opera scheme.** On Opera, Drop Suite adds an "Opera with Drop Suite" scheme
to the scheme selector: Opera's default look, with Drop Suite's primary and
highlight colours in matching maroon (`#6e0e0a`, hover `#580b08`, highlight
`#f8f3f3`). This is the recommended setup. Opera's other schemes reset the
Drop Suite colours to the blue defaults, so switching schemes never leaves
mismatched colours behind. Meaning colours (priority, due dates, success,
warning, danger) are the same in every scheme.

**Hard-coded colours.** Never write a palette colour as a bare hex value
(`color: #1565c0`); always use the property (`color: var(--drop-primary,
#1565c0)`), or the page will not follow the scheme.

**Priority colors (5 levels):**
| Level    | Badge color | Row background |
|----------|-------------|----------------|
| Critical | #7b0000     | #fff5f5        |
| High     | #c62828     | —              |
| Normal   | #1565c0     | #e3f2fd        |
| Low      | #757575     | #f5f5f5        |
| None     | #bdbdbd     | #f9f9f9        |

Properties: `--drop-priority-{level}-color`, `-bg`, `-text` (and
`--drop-priority-critical-row-bg`).

**Due date status:** `--drop-overdue-color` / `-bg`, `--drop-due-soon-color` /
`-bg`, `--drop-due-today-color` / `-bg`.
- Overdue: red (#c62828), bg #fff5f5
- Due soon: orange (#e65100), bg #ffe082
- Due today: orange (#e65100), bg #fff8e1

**Accent:**
- Teal left border on settings cards: `border-left: 3px solid #0891b2`

**Rules:**
- Use the custom properties, not raw hex values. Write them with the default
  as a fallback, e.g. `var(--drop-primary, #1565c0)`, so a module's CSS still
  works if it loads without `drop_suite.css`. The fallback must match the
  default in the table above.
- Only use names that are defined in `drop_suite.css`. Need a new colour? Add
  it to the `:root` block, to `drop_suite_palette()` and to the table above, in
  the same change. Don't invent a new name in a module's CSS.

## Icons

Use the Phosphor icons that ship with Backdrop core, through `icon()`:

```php
icon('warning', array('attributes' => array('width' => '16', 'height' => '16')));
```

Don't add icon fonts, image icons or other icon sets. Icon-only elements need
a text alternative: a `title` on the wrapper for the hover hint, plus an
`element-invisible` span for screen readers.

**Status flags** (a small marker that something needs attention, such as
overlapping time entries): an inline icon next to the item it describes,
coloured with `--drop-warning` (or `--drop-danger` for errors), with a `title`
that says what is wrong, e.g. "Overlaps with 10:30–11:15".

## Buttons

Opera (and several other themes) style default and `primary` submit buttons
the same, so use classes for visual hierarchy:

- The page-completion submit: default styling, plus `'#button_type' =>
  'primary'` for semantics.
- Auxiliary actions (Add row, Recalculate, Reset): add the `button-secondary`
  class so they recede.
- Destructive actions: add the `button-danger` class.

## Entity Edit Forms

Standards for all add/edit forms across the Drop Suite (tickets, tasks, contacts,
activities, projects, goals, time entries, docs).

### Page setup

1. **Narrow page width** — all entity add/edit pages are registered in
   `_drop_suite_is_narrow_page()` so they get the `drop-narrow-page` class.

2. **Breadcrumbs** — every entity form sets breadcrumbs. Pattern:
   - Add: `Home > Drop Suite > Section`
   - Edit: `Home > Drop Suite > Section > Entity title`

3. **Page title** — set via `backdrop_set_title()`. Add forms say
   "Add [type]"; edit forms say "Edit [type]".

4. **CSS attachment** — every form attaches its module CSS via
   `$form['#attached']['css'][]`. Never rely on CSS being loaded elsewhere.

### Layout

Two layout patterns depending on form complexity:

**Two-column layout** (for forms with many metadata fields — tickets, contacts):
```php
$form['layout'] = array(
  '#type' => 'container',
  '#attributes' => array('class' => array('drop-form-layout')),
);
$form['layout']['main'] = array(
  '#type' => 'container',
  '#attributes' => array('class' => array('drop-form-main')),
);
$form['layout']['sidebar'] = array(
  '#type' => 'container',
  '#attributes' => array('class' => array('drop-form-sidebar')),
);
```
- Main column: title, body/description, file attachments, tags, other large fields.
- Sidebar: type, status, priority, assignee, dates, and other compact metadata.
- Use `#type => 'container'` (not fieldsets, not `#prefix`/`#suffix` divs).
  Containers do not nest `$form_state['values']` — values stay at the top level.
- Shared CSS classes (`drop-form-layout`, `drop-form-main`, `drop-form-sidebar`)
  defined in `drop_suite.css` so all modules get the same flex layout and
  responsive breakpoint.

**Single-column layout** (for simple forms — goals, time entries):
- No layout containers needed. Fields render in weight order at full width.
- Make efficient use of space: put small fields next to each other horizontally
  where it makes sense. Lists of short items can be displayed horizontally.

### Field ordering

Use explicit `#weight` on every form element. Never rely on declaration order.

| Range    | Purpose                                          |
|----------|--------------------------------------------------|
| -20..-10 | Primary fields (title, body, type, project)      |
| -9..0    | Secondary fields (status, priority, assignee)    |
| 1..40    | Optional / conditional fields (tags, files, etc) |
| 41..60   | Reserved for form_alter additions                |
| 50       | Active/status checkbox (near bottom)             |
| 51       | Confirmation checkboxes (duplicate warnings)     |
| 99       | Actions / submit button (always last)            |

### Entity storage on the form

Store the entity being edited on the `$form` array with a `#` key named
after the entity type:
```php
$form['#ticket'] = $ticket;
$form['#project'] = $project;
$form['#goal'] = $goal;
```
Not in `$form_state` — the form array is the standard Backdrop pattern.

### Field API integration

Call `field_attach_form()` to allow Field UI fields to appear on the form.
If fields need to be repositioned into a layout column, use an `#after_build`
callback:
```php
$form['#after_build'][] = '_mymodule_form_move_fields';
```

### Actions and buttons

```php
$form['actions'] = array('#type' => 'actions', '#weight' => 99);
$form['actions']['submit'] = array(
  '#type'  => 'submit',
  '#value' => $is_edit ? t('Save') : t('Add @type', array('@type' => $type_label)),
);
$form['actions']['cancel'] = array(
  '#markup' => l(t('Cancel'), $cancel_path),
);
```

Rules:
- **Always** use `'#weight' => 99` on the actions element.
- **Always** include a Cancel link back to the listing page.
- **Button text**: sentence case — "Add ticket", "Save", "Add contact".
  Not title case ("Add Ticket").

### Current status

_Last checked April 2026; it has not been updated since and may be out of date. Check the code before relying on it._

| Form            | Module       | Breadcrumbs | Layout     | Actions #weight | Cancel | CSS attached |
|-----------------|--------------|-------------|------------|-----------------|--------|--------------|
| Ticket          | droptickets  | Yes         | Two-column | Needs fix       | Yes    | Yes          |
| Task            | droptasks    | No          | Single     | Needs fix       | Yes    | No           |
| Goal            | dropgoals    | No          | Single     | Needs fix       | Yes    | Yes          |
| Time Entry      | droptime     | No          | Single     | Needs fix       | Yes    | No           |
| Person          | drop_crm      | No          | Single     | Yes (99)        | No     | No           |
| Organization    | drop_crm      | No          | Single     | Yes (99)        | No     | No           |
| Household       | drop_crm      | No          | Single     | Yes (99)        | No     | No           |
| Activity        | drop_crm      | No          | Single     | Needs fix       | No     | No           |
| Project         | drop_suite    | No          | Single     | Needs fix       | Yes    | No           |

## Settings Pages

### Path convention

Two patterns, chosen by *scope of ownership*:

- **Module-owned settings** live in the module's own URL namespace:
  `drop/<module>/settings` for the primary form, and
  `drop/<module>/settings/<subpage>` for sub-settings
  (e.g. `drop/tickets/settings/statuses`). Use this for any setting
  that's only meaningful when the module is enabled — Drop CRM contact
  types, DropTickets statuses, DropBridge integration credentials.
  Keeps the module self-contained so uninstalling it doesn't leave
  orphan routes under `drop/settings/`.

- **Cross-cutting settings** live in the centralized namespace:
  `drop/settings/<thing>`. Use this only for things that affect
  multiple modules or the Drop Suite platform as a whole — module
  enable/disable (`drop/settings/modules`), shared page widths
  (`drop/settings/base`), primary nav customization
  (`drop/settings/navigation`).

**Drop Suite is a special case.** Because drop_suite provides platform-wide
surfaces (page widths, primary nav), its settings use the centralized
path shape: `drop/settings/base`, `drop/settings/navigation` — not
`drop/base/settings/*`. (The home-nav tool historically lived at
`drop/settings/home-nav`; it has since moved into the Drop Start setup
wizard at `drop/start/step/home_nav`. The old path still 301-redirects
for existing bookmarks.)

**Tools are not settings.** Generators, maintenance utilities, and
one-shot actions live outside `/settings/` entirely. Drop Start at
`drop/start` is a tool, not a settings surface, and should not be
retrofitted into the settings namespace.

**Dashboard cards link to top-level settings pages**, not deep sub-pages.
A card for DropTickets points to `drop/tickets/settings`; sub-pages like
status management are reached via in-page links or tabs from the
top-level page. Don't add a dashboard card for every sub-setting.

### Layout rules

1. **Every section is a collapsible fieldset, collapsed by default** —
   `#collapsible => TRUE` and `#collapsed => TRUE`, no exceptions. All
   fieldsets closed on load gives the user a full inventory of settings
   at a glance and keeps submodule-added sections on equal visual footing
   with the parent module's own sections. No bare headings, no raw markup
   sections outside a fieldset.

2. **Navigation links go elsewhere** — the settings page is for *settings*.
   Links to Field UI, status management pages, etc. belong in the settings
   dashboard (`drop/settings`) or in contextual links, not as fieldsets on
   the settings form.

3. **Submit button is always last** — use `'#weight' => 99` on the actions
   element so form_alter additions (from submodules) can slot in before it
   without needing to know the exact weight math.

4. **Simple forms still use fieldsets** — even a single checkbox should be
   inside a collapsible fieldset. This keeps the page visually consistent
   and scannable when more settings are added later.

### Fieldset template

```php
$form['section_name'] = array(
  '#type'        => 'fieldset',
  '#title'       => t('Section title'),
  '#description' => t('One sentence explaining what this group controls.'),
  '#collapsible' => TRUE,
  '#collapsed'   => TRUE,
  '#weight'      => 10,
);
$form['section_name']['my_setting'] = array(
  '#type'          => 'checkbox',
  '#title'         => t('Enable feature'),
  '#default_value' => $config->get('my_setting'),
);
```

### Weight conventions

| Range    | Purpose                                       |
|----------|-----------------------------------------------|
| -20..-10 | Core feature toggles (Features, Households)   |
| -5..0    | Primary settings (Outreach Goal, Activity Types) |
| 1..40    | Secondary / advanced settings                 |
| 41..60   | Reserved for form_alter additions (submodules) |
| 99       | Actions / submit button (always last)         |

### Submit button

```php
$form['actions'] = array('#type' => 'actions', '#weight' => 99);
$form['actions']['submit'] = array(
  '#type'  => 'submit',
  '#value' => t('Save settings'),
);
```

### Adding settings from a submodule

Use `hook_form_FORM_ID_alter()`. Pick a weight in the 41-60 range so the
section appears after the parent module's own settings but before the
submit button.

```php
function mysubmodule_form_parentmodule_settings_form_alter(&$form, &$form_state) {
  $form['my_section'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('My Section'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
    '#weight'      => 50,
  );
  // ...
  $form['#submit'][] = 'mysubmodule_settings_submit';
}
```

### Current status

_Last checked April 2026; it has not been updated since and may be out of date. Check the code before relying on it._

| Module       | Path                      | Follows guide? | Notes                          |
|--------------|---------------------------|----------------|--------------------------------|
| drop_suite    | drop/settings/base        | Yes            |                                |
| drop_crm      | drop/crm/settings         | Yes            |                                |
| dropbridge   | drop/settings/bridge      | No             | Module-owned; should be drop/bridge/settings |
| dropbridge   | drop/settings/inbound     | No             | Module-owned; should be drop/bridge/settings/inbound |
| dropemail    | drop/email/settings       | Yes            |                                |
| dropmonitor  | drop/monitor/settings     | Yes            |                                |
| droptickets  | drop/tickets/settings     | Mostly         | Status link is raw markup      |
| drop_docs     | drop/docs/settings        | Yes            |                                |

## Collapsible Details

Use native `<details>`/`<summary>` elements for content that should be
scannable but not always visible. Shared CSS classes live in `drop_suite.css`.

### Standard pattern

```html
<details class="drop-details">
  <summary class="drop-details-summary">Section title — brief status text</summary>
  <div class="drop-details-body">
    ...content...
  </div>
</details>
```

### Behavior

- **Collapsed by default** unless there is an actionable issue (error, outage,
  security update) — in that case add the `open` attribute.
- **Summary line** should include enough context that the person can decide
  whether to expand. Include a status indicator or count when available.
- **No JavaScript required** — `<details>` is natively interactive in all
  modern browsers.

### CSS classes

| Class                    | Purpose                                          |
|--------------------------|--------------------------------------------------|
| `.drop-details`          | Container — light background, border, radius     |
| `.drop-details-summary`  | Clickable header — uppercase label, arrow marker |
| `.drop-details-body`     | Inner content — padding, line-height             |
| `.drop-details-meta`     | Footer line — small, muted, links/timestamps     |

### When to use

- **Entity view pages** — collapsible info cards (site health, uptime, related
  data from other modules).
- **Dashboard cards** — expandable site lists, detail sections.
- **Admin pages** — optional detail that would clutter the page if always open.

### When NOT to use

- **Forms** — use `#type => 'fieldset'` with `#collapsible => TRUE` instead.
  Backdrop's FAPI handles form state persistence.
- **Primary content** — if the person always needs to see it, don't collapse it.

### Current usage

| Location                         | Class                        | Notes                        |
|----------------------------------|------------------------------|------------------------------|
| Project view: site health        | `drop-details`               | Opens if security update     |
| Project view: uptime             | `drop-details`               | Opens if site is down        |
| Monitor dashboard: site list     | `dropmonitor-sites-details`  | Module-specific (pre-pattern)|

## Entity View & Display Pattern

Custom entities in Drop Suite should behave like standard Backdrop entities:
fieldable, with Manage Display support, and listing pages as editable Views.

### Requirements

1. **Fieldable** — set `'fieldable' => TRUE` in `hook_entity_info()` with
   proper bundle admin paths for Field UI.
2. **Manage Display** — register pseudo-fields via `hook_field_extra_fields()`
   so module-rendered sections appear in the Manage Display form alongside
   Field API fields. Each section needs `label`, `description`, and `weight`.
3. **Respect display settings** — in the entity view callback, read display
   settings with `field_extra_fields_get_display()` and set `#weight` and
   `#access` on each render array element. Hidden fields get `#access = FALSE`.
4. **Views integration** — expose all schema columns via
   `hook_entity_property_info()` with correct types and getter callbacks.
   Entity Plus auto-generates Views handlers from these properties.
5. **Listing pages as Views** — ship a default view in
   `config/views.view.VIEWNAME.json`. Import in both `hook_install()` and an
   update hook. Remove hardcoded menu items for those paths so Views owns them.

### Implementation checklist

```
hook_entity_info()          → fieldable: TRUE, bundle admin paths
hook_field_extra_fields()   → register display pseudo-fields per bundle
hook_entity_property_info()  → expose all columns to Views
entity view callback        → field_extra_fields_get_display() for #weight/#access
config/views.view.*.json    → default listing view(s)
hook_install()              → import the view config
hook_update_NNNN()          → import the view config for existing sites
```

### Current status

_Last checked April 2026; it has not been updated since and may be out of date. Check the code before relying on it._

| Entity          | Module      | Fieldable | Extra fields | Properties | Views listing |
|-----------------|-------------|-----------|--------------|------------|---------------|
| Project         | drop_suite   | Yes       | Yes          | Yes        | Yes           |
| Person          | drop_crm     | Yes       | Yes          | Yes        | Yes           |
| Organization    | drop_crm     | Yes       | Yes          | Yes        | Yes           |
| Household       | drop_crm     | Yes       | Yes          | Yes        | Yes           |
| Activity        | drop_crm     | Yes       | No           | Yes        | No            |
| Contact Method  | drop_crm     | Yes       | No           | Yes        | No            |
| Ticket          | droptickets | Yes       | No           | Partial    | No            |
| Task            | droptasks   | Yes       | No           | Partial    | No            |
| Time Entry      | droptime    | Yes       | No           | Partial    | No            |

## Status Messages

**Any status message that references a piece of content must link to that
content.** The message is usually the admin's only confirmation that an
action succeeded — a plain name in italics is a dead end. The link lets
them jump straight to what they just touched.

```php
// Wrong:
backdrop_set_message(t('Project %name saved.', array(
  '%name' => $project->name,
)));

// Right:
backdrop_set_message(t('Project !link saved.', array(
  '!link' => l($project->name, 'drop/projects/' . $project->pid),
)));
```

Use the `!placeholder` form (not `%` or `@`) when the replacement value
is already-safe HTML from `l()`. `l()` handles URL and label escaping
internally; the `!` prefix tells `t()` to trust it.

**Exceptions — do NOT force a link when:**

- The entity has been **deleted** by the action ("File deleted" —
  nothing to link to). Use plain `%name` or omit the name entirely.
- The message references a **setting or configuration value** rather
  than an entity ("Navigation saved", "Statuses saved", "Settings saved").
- The message is **generic** about a batch operation ("@count items
  deleted") where no single item is meaningful.

For create/update/link operations — always link.
