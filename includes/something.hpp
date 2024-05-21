/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   something.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: declerbo <declerbo@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/21 14:23:19 by declerbo          #+#    #+#             */
/*   Updated: 2024/05/21 14:25:03 by declerbo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOMETHING_HPP
# define SOMETHING_HPP

#include <sys/socket.h>	// socket
#include <iostream>		// cout
#include <netinet/in.h> // struct sockaddr_in
#include <unistd.h> 	// close

// Number of connections allowed on the incoming queue (listen)
# define MAX_CLIENTS 50

#endif