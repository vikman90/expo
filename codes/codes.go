// Vikman Fernandez-Castro
// May 23, 2020

package codes

// Error codes
const (
	OK = iota
	Unauthorized
)

var messages = []string{
	"OK",
	"Unauthorized",
}

// Message returns the description of a code
func Message(code int) string {
	if code >= 0 && code < len(messages) {
		return messages[code]
	} else {
		return "Unknown"
	}
}
